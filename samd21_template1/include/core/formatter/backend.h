/*
 * backend.h
 *
 * Created: 25.8.2026 9.34.25
 *  Author: Kari63
 */ 

#pragma once

#include <cstdint>


// Formatter backend interface
void fb_putc(std::uint8_t backend, char c);

// TFT driver
std::uint8_t TFT_drawChar(std::int16_t x, std::int16_t y, unsigned char c);
std::uint16_t TFT_drawString(const char *s);

// LCD driver
void lcd_putc(char c);
void lcd_puts(const char *s);

// SERCOM2 / USART2
void usart2_write_char(std::uint8_t c);
void usart2_write_string(const char *s);

// usart4 / USART4 (if used)
void usart4_write_char(const char *s);
void usart4_write_string(const char *s);



