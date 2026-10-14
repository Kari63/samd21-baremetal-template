/*
 * usart4.h
 *
 * Created: 15.8.2026 15.55.35
 *  Author: Kari63
 */ 


#pragma once

#include <stdint.h>
#include <stdbool.h>

int  usart4_read_char(uint8_t *out);
void usart4_write_char(uint8_t c);
void usart4_write_string(const char *s);
void usart4_write_int(int16_t v);
void usart4_write_hex(uint8_t v);
void usart4_write_crlf(void);
