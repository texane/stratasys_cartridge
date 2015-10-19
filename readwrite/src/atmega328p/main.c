#include <stdint.h>
#include "uart.c"


/* ds2433 static configuration */

#define DS2433_DATA_MASK (1 << 2)
#define DS2433_DATA_DDR (DDRD)
#define DS2433_DATA_PORT (PORTD)
#define DS2433_DATA_PIN (PIND)
#include "ds2433.c"


/* global variable */

static uint8_t rom_buf[DS2433_ROM_SIZE];
static uint8_t mem_buf[DS2433_MEM_SIZE];



/* line related routines */

#define LINE_MAX_SIZE 16

static uint8_t is_eol(char c)
{
  return (c == '\r') || (c == '\n');
}

static void write_eol(void)
{
  uart_write_rn();
}

static void write_buf(uint8_t* buf, uint16_t size)
{
  uint16_t i;

  for (i = 0; i != size; ++i, ++buf)
  {
    if (i && (((uint8_t)i & (16 - 1)) == 0)) write_eol();
    uart_write(uint8_to_string(*buf), 2);
  }

  write_eol();
}

static void write_ok(void)
{
  UART_WRITE_STRING("ok");
  write_eol();
}

static void write_ko(void)
{
  UART_WRITE_STRING("ko");
  write_eol();
}

static uint8_t read_line(char* line, uint8_t len)
{
  uint8_t i;

  for (i = 0; i != len; ++i, ++line)
  {
    uart_read_uint8((uint8_t*)line);
    if (is_eol(*line)) break ;
  }

  return i;
}

static void skip_ws(const char** line, uint8_t* len)
{
  for (; *len && (**line == ' '); --*len, ++*line) ;
}


/* command handlers */

static uint16_t mem_addr = 0;

static void do_addr(const char* line, uint8_t len)
{
  uint16_t tmp;

  skip_ws(&line, &len);

  if (len == 0)
  {
    /* get address value */
    write_ok();
    uart_write((const uint8_t*)uint16_to_string(mem_addr), 4);
    write_eol();
  }
  else
  {
    /* set address value */
    if (string_to_uint16(line, &tmp)) goto on_error;
    mem_addr = tmp;
    write_ok();
  }

  return ;

 on_error:
  write_ko();
  return ;
}

static void do_rmem(const char* line, uint8_t len)
{
  uint16_t size;

  if (len != 5) goto on_error;

  --len;
  ++line;

  if (string_to_uint16(line, &size)) goto on_error;
  if ((mem_addr + size) > DS2433_MEM_SIZE) goto on_error;
  if (ds2433_read_mem(mem_buf, mem_addr, size)) goto on_error;
  mem_addr += size;

  write_ok();
  write_buf(mem_buf, size);
  return ;

 on_error:
  write_ko();
  return ;
}

static void do_rrom(const char* line, uint8_t len)
{
  if (len) goto on_error;

  if (ds2433_read_rom(rom_buf))
  {
    write_ko();
    return ;
  }

  write_ok();
  write_buf(rom_buf, DS2433_ROM_SIZE);
  return ;

 on_error:
  write_ko();
  return ;
}

static void do_wmem(const char* line, uint8_t len)
{
  /* not implemented */

  write_ko();
}


/* main */

static uint8_t is_eq(const char* a, const char* b, uint8_t len)
{
  for (; len && (*a == *b); --len, ++a, ++b) ;
  return len == 0;
}

int main(int ac, char** av)
{
#define DEFINE_HANDLER(__s) { #__s, sizeof(#__s) - 1, do_ ## __s }
  static struct
  {
    const char* s;
    uint8_t len;
    void (*f)(const char*, uint8_t);
  } h[] =
  {
    DEFINE_HANDLER(addr),
    DEFINE_HANDLER(rmem),
    DEFINE_HANDLER(wmem),
    DEFINE_HANDLER(rrom)
  };

  static const uint8_t n = sizeof(h) / sizeof(h[0]);

  char line_buf[LINE_MAX_SIZE];
  uint8_t i;
  uint8_t len;

  uart_setup();

  ds2433_setup();

  while (1)
  {
    len = read_line(line_buf, sizeof(line_buf));
    if (len == 0) continue ;

    if (len == LINE_MAX_SIZE) continue ;

    for (i = 0; i != n; ++i)
    {
      if (len < h[i].len) continue ;
      if (is_eq(line_buf, h[i].s, h[i].len)) break ;
    }

    if (i != n) h[i].f(line_buf + h[i].len, len - h[i].len);
    else write_ko();
  }

  return 0;
}
