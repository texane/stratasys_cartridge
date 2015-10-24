#ifndef DS2433_C_INCLUDED
#define DS2433_C_INCLUDED


/* ds2433 communication routines */


#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>


/* check required macros */

#ifndef DS2433_DATA_MASK
#error "DS2433_DATA_MASK" not defined
#endif /* DS2433_DATA_MASK */

#ifndef DS2433_DATA_DDR
#error "DS2433_DATA_DDR" not defined
#endif /* DS2433_DATA_DDR */

#ifndef DS2433_DATA_PORT
#error "DS2433_DATA_PORT" not defined
#endif /* DS2433_DATA_PORT */

#ifndef DS2433_DATA_PIN
#error "DS2433_DATA_PIN" not defined
#endif /* DS2433_DATA_PIN */


/* onewire master static configuration */

#define OW_MASTER_DATA_MASK DS2433_DATA_MASK
#define OW_MASTER_DATA_DDR DS2433_DATA_DDR
#define OW_MASTER_DATA_PORT DS2433_DATA_PORT
#define OW_MASTER_DATA_PIN DS2433_DATA_PIN
#include "ow_master.c"


#define DS2433_ERR_SUCCESS ((uint8_t)0)
#define DS2433_ERR_FAILURE ((uint8_t)-1)


static void ds2433_setup(void)
{
  ow_master_setup();
}


static void ds2433_skip_rom(void)
{
  /* ds2433.pdf, p.12 */
  /* used to skip addressing phase in single device bus */

  ow_master_write_uint8(0xcc);
}


static uint8_t ds2433_read_rom(uint8_t* buf)
{
  /* ds2433.pdf, p.12 */
  /* rom is the device full 64 bits identifier */

#define DS2433_ROM_SIZE 8

  if (ow_master_reset_slave()) return (uint8_t)-1;
  ow_master_write_uint8(0x33);
  ow_master_read(buf, DS2433_ROM_SIZE);
  return 0;
}


static uint8_t ds2433_cmd_norom(uint8_t cmd)
{
  /* issue a romless command */
  /* reset, skip rom, issue command */

  if (ow_master_reset_slave()) return DS2433_ERR_FAILURE;

  ds2433_skip_rom();
  ow_master_write_uint8(cmd);
  return DS2433_ERR_SUCCESS;
}


static uint8_t ds2433_read_mem
(uint8_t* buf, uint16_t addr, uint16_t size)
{
  /* ds2433.pdf, p.7 */

  uint16_t i;

  if (ds2433_cmd_norom(0xf0)) return DS2433_ERR_FAILURE;

  /* 2 byte target address, LSByte first */
  ow_master_write_uint8((uint8_t)((addr >> 0) & 0xff));
  ow_master_write_uint8((uint8_t)((addr >> 8) & 0xff));

  for (i = 0; i != size; ++i, ++buf) ow_master_read_uint8(buf);

  return 0;
}


static uint8_t ds2433_read_all_mem(uint8_t* buf)
{
#define DS2433_MEM_SIZE 512
  return ds2433_read_mem(buf, 0, DS2433_MEM_SIZE);
}


static uint8_t ds2433_write_scratchpad
(const uint8_t* buf, uint16_t addr, uint16_t size)
{
  uint16_t i;

  if (ds2433_cmd_norom(0x0f)) return DS2433_ERR_FAILURE;

  /* 2 byte target address, LSByte first */
  ow_master_write_uint8((uint8_t)((addr >> 0) & 0xff));
  ow_master_write_uint8((uint8_t)((addr >> 8) & 0xff));

  for (i = 0; i != size; ++i, ++buf) ow_master_write_uint8(*buf);

  return DS2433_ERR_SUCCESS;
}


static uint8_t ds2433_read_scratchpad(uint8_t* aa)
{
#define DS2433_AA_SIZE 3

  uint8_t i;

  if (ds2433_cmd_norom(0xaa)) return DS2433_ERR_FAILURE;

  /* read authorization code */
  for (i = 0; i != DS2433_AA_SIZE; ++i, ++aa) ow_master_read_uint8(aa);

  /* skip data bytes that follow */
  /* verification done by host software */

  return 0;
}


static uint8_t ds2433_copy_scratchpad(const uint8_t* aa)
{
  uint8_t i;

  if (ds2433_cmd_norom(0x55)) return DS2433_ERR_FAILURE;

  for (i = 0; i != DS2433_AA_SIZE; ++i, ++aa) ow_master_write_uint8(*aa);

  ow_master_release_data();
  _delay_ms(5);

  return DS2433_ERR_SUCCESS;
}


static uint8_t ds2433_write_mem
(const uint8_t* buf, uint16_t addr, uint16_t size)
{
  /* ds2433.pdf, p.10 */
  /* write, read, copy scratchpad */

  uint8_t aa[DS2433_AA_SIZE];

  if (ds2433_write_scratchpad(buf, addr, size)) goto on_error;
  if (ds2433_read_scratchpad(aa)) goto on_error;
  if (ds2433_copy_scratchpad(aa)) goto on_error;

  return DS2433_ERR_SUCCESS;

 on_error:
  return DS2433_ERR_FAILURE;
}


#endif /* DS2433_C_INCLUDED */
