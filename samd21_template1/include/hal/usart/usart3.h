/* usart3.h - SERCOM3 USART = console (EDBG virtual COM, PA22 TX / PA23 RX, 115200) */
/*
 * usart3.h
 *
 * Created: 15.8.2026 15.55.35
 *  Author: Kari63
 */ 


#pragma once

#include <stdint.h>
#include <stdbool.h>

int  usart3_read_char(uint8_t *out);
void usart3_write_char(uint8_t c);
void usart3_write_string(const char *s);
void usart3_write_int(int16_t v);
void usart3_write_hex(uint8_t v);
void usart3_write_crlf(void);
