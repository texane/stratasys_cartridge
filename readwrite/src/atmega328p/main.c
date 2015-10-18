#include <stdint.h>
#include "uart.c"

/* ds2433 static configuration */
#define DS2433_DATA_MASK (1 << 2)
#define DS2433_DATA_DDR (DDRD)
#define DS2433_DATA_PORT (PORTD)
#define DS2433_DATA_PIN (PIND)
#include "ds2433.c"


static void print_buf(uint8_t* buf, uint16_t size)
{
  uint16_t i;

  for (i = 0; i != size; ++i, ++buf)
  {
    if ((((uint8_t)i) & (16 - 1)) == 0)
    {
      uart_write_rn();
    }

    uart_write(uint8_to_string(*buf), 2);
  }

  uart_write_rn();
}


int main(int ac, char** av)
{
  static uint8_t rom_buf[DS2433_ROM_SIZE];
  static uint8_t mem_buf[DS2433_MEM_SIZE];

  uart_setup();
  UART_WRITE_STRING("starting\r\n");

  ds2433_setup();

  while (1)
  {
    _delay_ms(500);

    ds2433_read_rom(rom_buf);
    ds2433_read_all_mem(mem_buf);

    print_buf(rom_buf, DS2433_ROM_SIZE);
    print_buf(mem_buf, sizeof(mem_buf));
  }

  return 0;
}
