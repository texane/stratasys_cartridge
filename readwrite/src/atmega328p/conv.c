#ifndef CONV_C_INCLUDED
#define CONV_C_INCLUDED


/* string to from uint conversion routines */


#include <stdint.h>


/* uintx_to_string */

static inline uint8_t nibble(uint32_t x, uint8_t i)
{
  return (x >> (i * 4)) & 0xf;
}

static inline uint8_t hex(uint8_t x)
{
  return (x >= 0xa) ? 'a' + x - 0xa : '0' + x;
}

static uint8_t hex_buf[8];

static uint8_t* uint8_to_string(uint8_t x)
{
  hex_buf[1] = hex(nibble(x, 0));
  hex_buf[0] = hex(nibble(x, 1));

  return hex_buf;
}

static uint8_t* uint16_to_string(uint16_t x)
{
  hex_buf[3] = hex(nibble(x, 0));
  hex_buf[2] = hex(nibble(x, 1));
  hex_buf[1] = hex(nibble(x, 2));
  hex_buf[0] = hex(nibble(x, 3));

  return hex_buf;
}

static uint8_t* uint32_to_string(uint32_t x)
{
  hex_buf[7] = hex(nibble(x, 0));
  hex_buf[6] = hex(nibble(x, 1));
  hex_buf[5] = hex(nibble(x, 2));
  hex_buf[4] = hex(nibble(x, 3));
  hex_buf[3] = hex(nibble(x, 4));
  hex_buf[2] = hex(nibble(x, 5));
  hex_buf[1] = hex(nibble(x, 6));
  hex_buf[0] = hex(nibble(x, 7));

  return hex_buf;
}


/* string_to_uintx */

static uint8_t char_to_uint8(char c, uint8_t* x)
{
  char xx;
  
  if ((c >= '0') && (c <= '9')) xx = '0';
  else if ((c >= 'a') && (c <= 'f')) xx = 'a';
  else if ((c >= 'A') && (c <= 'F')) xx = 'A';
  else return (uint8_t)-1;
  
  *x = c - xx;

  return 0;
}

static uint8_t string_to_uint8(const char* s, uint8_t* x)
{
  uint8_t tmp;

  *x = 0;

  if (char_to_uint8(s[0], &tmp)) return (uint8_t)-1;
  *x |= tmp << 4;

  if (char_to_uint8(s[1], &tmp)) return (uint8_t)-1;
  *x |= tmp << 0;

  return 0;
}

static uint8_t string_to_uint16(const char* s, uint16_t* x)
{
  uint8_t tmp;

  *x = 0;

  if (char_to_uint8(s[0], &tmp)) return (uint8_t)-1;
  *x |= (uint16_t)tmp << 12;

  if (char_to_uint8(s[1], &tmp)) return (uint8_t)-1;
  *x |= (uint16_t)tmp << 8;

  if (char_to_uint8(s[2], &tmp)) return (uint8_t)-1;
  *x |= (uint16_t)tmp << 4;

  if (char_to_uint8(s[3], &tmp)) return (uint8_t)-1;
  *x |= (uint16_t)tmp << 0;

  return 0;
}


#endif /* CONV_C_INCLUDED */
