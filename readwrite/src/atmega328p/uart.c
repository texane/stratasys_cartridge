#ifndef UART_C_INCLUDED
#define UART_C_INCLUDED


#include <stdint.h>
#include <avr/io.h>


static inline void set_baud_rate(long baud)
{
#ifndef CLK_PRESCAL
#define CLK_PRESCAL (1UL)
#endif

  uint16_t x = ((F_CPU / (16UL * CLK_PRESCAL) + baud / 2) / baud - 1);
  UBRR0H = x >> 8;
  UBRR0L = x;
}

static void uart_setup(void)
{
#if (CLK_PRESCAL == 1UL)
  set_baud_rate(9600);
#else
  set_baud_rate(300);
#endif

  /* baud doubler off  - Only needed on Uno XXX */
  UCSR0A &= ~(1 << U2X0);

  UCSR0B = (1 << RXEN0) | (1 << TXEN0);

  /* default to 8n1 framing */
  UCSR0C = (3 << 1);
}

static void uart_write(const uint8_t* s, uint8_t n)
{
  for (; n; --n, ++s)
  {
    /* wait for transmit buffer to be empty */
    while (!(UCSR0A & (1 << UDRE0))) ;
    UDR0 = *s;
  }

  /* wait for last byte to be sent */
  while ((UCSR0A & (1 << TXC0)) == 0) ;
}

static uint8_t uart_read_uint8(uint8_t* x)
{
  /* return non zero on error */

  uint8_t err = 0;

  while ((UCSR0A & (1 << RXC0)) == 0) ;

  if (UCSR0A & ((1 << FE0) | (1 << DOR0) | (1 << UPE0)))
  {
    /* clear errors by reading URD0 */
    err = 1;
  }

  *x = UDR0;

  return err;
}

static void uart_flush_rx(void)
{
  volatile uint8_t x;

  while (UCSR0A & (1 << RXC0))
  {
    x = UDR0;
    __asm__ __volatile__ ("nop" ::"m"(x));
  }
}


/* string related helper routines */

static void uart_write_rn(void)
{
  uart_write((uint8_t*)"\r\n", 2);
}

#define UART_WRITE_STRING(__s)			\
do {						\
  uart_write((uint8_t*)__s, sizeof(__s) - 1);	\
} while (0)


#endif /* UART_C_INCLUDED */
