#ifndef OW_MASTER_C_INCLUDED
#define OW_MASTER_C_INCLUDED


/* one wire master */
/* bitanged implementation */
/* regular speed (ie. not overdrive) */
/* https://www.maximintegrated.com/en/app-notes/index.mvp/id/126 */
/* https://en.wikipedia.org/wiki/1-Wire */


#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>


/* check required macros */

#ifndef OW_MASTER_DATA_MASK
#error "OW_MASTER_DATA_MASK" not defined
#endif /* OW_MASTER_DATA_MASK */

#ifndef OW_MASTER_DATA_DDR
#error "OW_MASTER_DATA_DDR" not defined
#endif /* OW_MASTER_DATA_DDR */

#ifndef OW_MASTER_DATA_PORT
#error "OW_MASTER_DATA_PORT" not defined
#endif /* OW_MASTER_DATA_PORT */

#ifndef OW_MASTER_DATA_PIN
#error "OW_MASTER_DATA_PIN" not defined
#endif /* OW_MASTER_DATA_PIN */


static void ow_master_setup(void)
{
  OW_MASTER_DATA_DDR |= OW_MASTER_DATA_MASK;
  OW_MASTER_DATA_PORT |= OW_MASTER_DATA_MASK;
}


static uint8_t ow_master_get_data(void)
{
  return OW_MASTER_DATA_PIN & OW_MASTER_DATA_MASK;
}


static void ow_master_set_data_low(void)
{
  OW_MASTER_DATA_PORT &= ~OW_MASTER_DATA_MASK;
}


static void ow_master_set_data_high(void)
{
  OW_MASTER_DATA_PORT |= OW_MASTER_DATA_MASK;
}


static void ow_master_acquire_data(void)
{
  /* set output state */
  OW_MASTER_DATA_DDR |= OW_MASTER_DATA_MASK;
}


static void ow_master_release_data(void)
{
  /* set tristate */
  /* doc8161.pdf, p77 */
  /* {DDxn, PORTxn} = 0b00 */

  OW_MASTER_DATA_PORT &= ~OW_MASTER_DATA_MASK;
  OW_MASTER_DATA_DDR &= ~OW_MASTER_DATA_MASK;
}


static uint8_t ow_master_reset_slave(void)
{
  /* reset slave */
  ow_master_acquire_data();
  ow_master_set_data_low();
  _delay_us(480);

  ow_master_release_data();
  _delay_us(70);

  /* detect slave presence bit */
  if (ow_master_get_data()) return 1;

  /* wait for slave to release the line */
  _delay_us(410);

  return 0;
}


static uint8_t ow_master_read_bit(void)
{
  uint8_t res;

  ow_master_acquire_data();
  OW_MASTER_DATA_PORT &= ~OW_MASTER_DATA_MASK;
  _delay_us(6);

  ow_master_release_data();
  _delay_us(9);

  if (OW_MASTER_DATA_PIN & OW_MASTER_DATA_MASK) res = 1;
  else res = 0;

  _delay_us(55);

  return res;
}


static void ow_master_read_uint8(uint8_t* x)
{
  *x = 0;

  *x |= ow_master_read_bit() << 0;
  *x |= ow_master_read_bit() << 1;
  *x |= ow_master_read_bit() << 2;
  *x |= ow_master_read_bit() << 3;
  *x |= ow_master_read_bit() << 4;
  *x |= ow_master_read_bit() << 5;
  *x |= ow_master_read_bit() << 6;
  *x |= ow_master_read_bit() << 7;
}


static void ow_master_read(uint8_t* x, uint8_t n)
{
  /* read data from slave */

  uint8_t i;
  for (i = 0; i != n; ++i, ++x) ow_master_read_uint8(x);
}


static void ow_master_write_zero(void)
{
  OW_MASTER_DATA_PORT &= ~OW_MASTER_DATA_MASK;
  _delay_us(60);
  OW_MASTER_DATA_PORT |= OW_MASTER_DATA_MASK;
  _delay_us(10);
}


static void ow_master_write_one(void)
{
  OW_MASTER_DATA_PORT &= ~OW_MASTER_DATA_MASK;
  _delay_us(6);
  OW_MASTER_DATA_PORT |= OW_MASTER_DATA_MASK;
  _delay_us(64);
}


#define OW_MASTER_SEND_BIT(__x, __i)			\
do {							\
  if ((__x) & (1 << (__i))) ow_master_write_one();	\
  else ow_master_write_zero();				\
} while (0)


static void ow_master_write_uint8(uint8_t x)
{
  ow_master_acquire_data();

  OW_MASTER_SEND_BIT(x, 0);
  OW_MASTER_SEND_BIT(x, 1);
  OW_MASTER_SEND_BIT(x, 2);
  OW_MASTER_SEND_BIT(x, 3);
  OW_MASTER_SEND_BIT(x, 4);
  OW_MASTER_SEND_BIT(x, 5);
  OW_MASTER_SEND_BIT(x, 6);
  OW_MASTER_SEND_BIT(x, 7);
}


#endif /* OW_MASTER_C_INCLUDED */
