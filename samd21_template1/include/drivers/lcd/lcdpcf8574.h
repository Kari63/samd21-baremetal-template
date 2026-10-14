/*
 * lcdpcf8574.h - HD44780 character LCD via PCF8574 I2C backpack (4-bit)
 *
 * Copyright 2026 Kari63
 * SPDX-License-Identifier: Apache-2.0
 *
 * Written from the HD44780U and PCF8574 datasheets. Write-only (RW = 0):
 * the instruction execution time is covered by the I2C frame timing, which
 * is checked at compile time; clear/home is waited with a cycle delay.
 *
 * Shadow buffer: lcd_clrscr/home/gotoxy/putc/puts write to the buffer
 * only. lcd_flush() sends only the changed characters to the display.
 * lcd_command() and lcd_data() are raw HW functions that bypass the
 * buffer (display contents and shadow may differ after them).
 * lcd_invalidate() forces the next flush to write the whole display;
 * the application calls it periodically to repair disturbances.
 *
 * Do not call these functions from an interrupt (polled I2C).
 */

#pragma once

#include <stdint.h>


/* --------------------------------------------------------------------
   CONFIGURATION
-------------------------------------------------------------------- */
#define LCD_PCF8574_DEVICEID    7       /* A2..A0 = 111 -> address 0x27 */
#define LCD_LINES               2       /* 1..4 */
#define LCD_COLUMNS             16


/* --------------------------------------------------------------------
   TIMING
-------------------------------------------------------------------- */
#define LCD_CPU_MHZ             48      /* SystemInit: DFLL48M -> GCLK0 */
#define LCD_I2C_KHZ             400     /* SERCOM0 BAUD=59: nominal ~400 kHz,
                                           actual ~340 kHz. The fastest value
                                           is the safest (shortest byte time). */
#define LCD_EXEC_US             53      /* HD44780U instruction/data, fosc min 190 kHz
                                           (nominal 37 us @ 270 kHz) */
#define LCD_CLEAR_US            2200    /* clear/home: 1.52 ms @ 270 kHz,
                                           ~2.16 ms @ 190 kHz */


/* --------------------------------------------------------------------
   PCF8574 -> HD44780 WIRING (common I2C backpack)
   D4..D7 are on consecutive pins starting at LCD_PIN_D4.
-------------------------------------------------------------------- */
#define LCD_PIN_RS      0
#define LCD_PIN_RW      1
#define LCD_PIN_E       2
#define LCD_PIN_BL      3
#define LCD_PIN_D4      4


/* --------------------------------------------------------------------
   dispAttr for lcd_init() ("Display on/off control" instruction)
-------------------------------------------------------------------- */
#define LCD_DISP_OFF                0x08
#define LCD_DISP_ON                 0x0C
#define LCD_DISP_ON_BLINK           0x0D
#define LCD_DISP_ON_CURSOR          0x0E
#define LCD_DISP_ON_CURSOR_BLINK    0x0F


/* --------------------------------------------------------------------
   API
-------------------------------------------------------------------- */
void    lcd_init(uint8_t dispAttr);
void    lcd_clrscr(void);
void    lcd_home(void);
void    lcd_gotoxy(uint8_t x, uint8_t y);
void    lcd_led(uint8_t onoff);
void    lcd_putc(char c);
void    lcd_puts(const char *s);
void    lcd_flush(void);
void    lcd_invalidate(void);

void    lcd_command(uint8_t cmd);
void    lcd_data(uint8_t data);
void    lcd_clear_CGRAM(void);
uint8_t lcd_create_custom_char(uint8_t charnum, const uint8_t *values);
