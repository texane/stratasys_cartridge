#ifndef DS2433_C_INCLUDED
#define DS2433_C_INCLUDED


/* ds2433 communication routines */


#include <stdint.h>
#include <avr/io.h>


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


static void ds2433_setup(void)
{
  ow_master_setup();
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


static void ds2433_skip_rom(void)
{
  /* ds2433.pdf, p.12 */
  /* used to skip addressing phase in single device bus */

  ow_master_write_uint8(0xcc);
}


static uint8_t ds2433_read_mem
(uint8_t* buf, uint16_t addr, uint16_t size)
{
  /* ds2433.pdf, p.7 */

  uint16_t i;

  if (ow_master_reset_slave()) return (uint8_t)-1;

  ds2433_skip_rom();

  /* read memory command */
  ow_master_write_uint8(0xf0);

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


#endif /* DS2433_C_INCLUDED */
