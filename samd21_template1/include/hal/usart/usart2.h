/*
 * usart2.h
 *
 * Created: 17.8.2026 15.44.31
 *  Author: Kari63
 */ 


#pragma once

#include <stdint.h>
#include <stdbool.h>

int  usart2_read_char(uint8_t *out);
void usart2_write_char(uint8_t c);
void usart2_write_string(const char *s);
void usart2_write_int(int16_t v);
void usart2_write_hex(uint8_t v);
void usart2_write_crlf(void);