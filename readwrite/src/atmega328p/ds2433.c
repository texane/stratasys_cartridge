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


/* sizes */

#define DS2433_AA_SIZE 3
#define DS2433_SPAD_SIZE 32
#define DS2433_ROM_SIZE 8
#define DS2433_MEM_SIZE 512


/* error code */

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
  return ds2433_read_mem(buf, 0, DS2433_MEM_SIZE);
}


static uint8_t ds2433_write_spad_safe
(const uint8_t* buf, uint16_t addr, uint8_t size)
{
  /* safe version, arguments validated */

  uint8_t i;

  if (ds2433_cmd_norom(0x0f)) return DS2433_ERR_FAILURE;

  /* 2 byte target address, LSByte first */
  ow_master_write_uint8((uint8_t)((addr >> 0) & 0xff));
  ow_master_write_uint8((uint8_t)((addr >> 8) & 0xff));

  for (i = 0; i != size; ++i, ++buf) ow_master_write_uint8(*buf);

  return DS2433_ERR_SUCCESS;
}


static uint8_t ds2433_write_spad
(const uint8_t* buf, uint16_t addr, uint8_t size)
{
  if (size >= DS2433_SPAD_SIZE) return DS2433_ERR_FAILURE;
  return ds2433_write_spad_safe(buf, addr, size);
}


static uint8_t ds2433_read_spad(uint8_t* aa)
{
  uint8_t i;

  if (ds2433_cmd_norom(0xaa)) return DS2433_ERR_FAILURE;

  /* read authorization code */
  for (i = 0; i != DS2433_AA_SIZE; ++i, ++aa) ow_master_read_uint8(aa);

  /* skip data bytes that follow */

  return 0;
}


static uint8_t ds2433_copy_spad(const uint8_t* aa)
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
  uint8_t i;
  uint8_t ii;

  /* align first access relative to DS2433_SPAD_SIZE */

  i = (uint8_t)addr % DS2433_SPAD_SIZE;
  if (i)
  {
    i = DS2433_SPAD_SIZE - i;

    if (ds2433_write_spad_safe(buf, addr, i)) goto on_error;
    if (ds2433_read_spad(aa)) goto on_error;
    if (ds2433_copy_spad(aa)) goto on_error;

    buf += i;
    addr += (uint16_t)i;
    size += (uint16_t)i;
  }

  /* write entire DS2433_SPAD_SIZE blocks */

  ii = (uint8_t)(size / (uint16_t)DS2433_SPAD_SIZE);
  for (i = 0; i != ii; ++i)
  {
    if (ds2433_write_spad_safe(buf, addr, DS2433_SPAD_SIZE)) goto on_error;
    if (ds2433_read_spad(aa)) goto on_error;
    if (ds2433_copy_spad(aa)) goto on_error;

    buf += DS2433_SPAD_SIZE;
    addr += DS2433_SPAD_SIZE;
    size -= DS2433_SPAD_SIZE;
  }

  /* write remainder */

  if (size)
  {
    if (ds2433_write_spad_safe(buf, addr, (uint8_t)size)) goto on_error;
    if (ds2433_read_spad(aa)) goto on_error;
    if (ds2433_copy_spad(aa)) goto on_error;
  }

  return DS2433_ERR_SUCCESS;

 on_error:
  return DS2433_ERR_FAILURE;
}


#endif /* DS2433_C_INCLUDED */
