/*
 * lcdpcf8574.cpp - HD44780 character LCD via PCF8574 I2C backpack (4-bit)
 *
 * Copyright 2026 Kari63
 * SPDX-License-Identifier: Apache-2.0
 *
 * One LCD byte = one I2C frame:
 *
 *   [address] ([setup]) [hi|E] [hi] [lo|E] [lo]
 *
 * - The PCF8574 updates its outputs at the ACK of every data byte, so each
 *   byte is one pin state and states are one byte time apart (9 bits,
 *   ~22 us @ 400 kHz). PWEH (450 ns) and tDSW (195 ns) are met with margin.
 * - A nibble takes two states: data changes on the rising edge of E and is
 *   latched on the falling edge. The RS setup state (tAS) is added only when
 *   RS changes.
 * - Command execution time: from the last falling E edge of one byte to the
 *   first falling E edge of the next there is at least STOP + START +
 *   address + 2 states, i.e. > 3 byte times. Checked with static_assert.
 * - Clear/home (> 1.52 ms) is waited with a cycle-counted delay. The HW
 *   clear is done only in init; lcd_clrscr() clears the buffer only.
 *
 * Shadow buffer: frame[] = wanted view, shadow[] = what is on the display.
 * lcd_flush() sends the differences and skips the address command when the
 * auto-incremented HD44780 DDRAM address is already correct.
 */

#include "project_config.h"

#if USE_LCD

#include "lcdpcf8574.h"
#include "pcf8574.h"
#include "systick.h"


static_assert(LCD_LINES >= 1 && LCD_LINES <= 4, "LCD_LINES 1..4");
static_assert(LCD_PIN_D4 <= 4, "D4..D7 do not fit into the PCF8574 pins");
static_assert(LCD_CPU_MHZ % 3 == 0, "lcd_delay_us: LCD_CPU_MHZ must be divisible by 3");

/* 3 byte times (9 bits/byte) >= command execution time */
static_assert(27000000UL / LCD_I2C_KHZ >= LCD_EXEC_US * 1000UL,
              "I2C too fast: frame gap does not cover the HD44780 execution time");


/* HD44780U instructions (datasheet, table 6) */
static constexpr uint8_t HD_CLEAR       = 0x01;
static constexpr uint8_t HD_HOME        = 0x02;     /* bit 0 = don't care */
static constexpr uint8_t HD_ENTRY_INC   = 0x06;     /* I/D=1, S=0 */
static constexpr uint8_t HD_FUNC_4BIT   = 0x20;     /* DL=0, N=0, F=0 */
static constexpr uint8_t HD_FUNC_2LINE  = 0x08;     /* N=1 */
static constexpr uint8_t HD_SET_CGRAM   = 0x40;
static constexpr uint8_t HD_SET_DDRAM   = 0x80;

/* PCF8574 bits. RW is always kept at 0. */
static constexpr uint8_t BIT_RS    = 1u << LCD_PIN_RS;
static constexpr uint8_t BIT_RW    = 1u << LCD_PIN_RW;
static constexpr uint8_t BIT_E     = 1u << LCD_PIN_E;
static constexpr uint8_t BIT_BL    = 1u << LCD_PIN_BL;

/* DDRAM start address of each line. Lines 3 and 4 continue lines 1 and 2. */
static const uint8_t line_start[4] = {
	0x00, 0x40, LCD_COLUMNS, 0x40 + LCD_COLUMNS
};

static constexpr uint8_t HW_ADDR_UNKNOWN = 0xFF;

static uint8_t lcd_bl;          /* BIT_BL or 0 */
static uint8_t cur_x;           /* buffer cursor */
static uint8_t cur_y;

/* frame = wanted view, shadow = what is on the display. lcd_flush() sends the differences. */
static char    frame[LCD_LINES][LCD_COLUMNS];
static char    shadow[LCD_LINES][LCD_COLUMNS];
static uint8_t hw_addr;         /* HD44780 DDRAM address counter, if known */
static bool    shadow_valid;    /* false: next flush writes everything */


/* Cycle delay: SUBS (1) + taken BNE (2) = 3 cycles/iteration without wait
   states. Flash wait states and interrupts only make it longer, so the delay
   is a lower bound. The asm loop does not depend on the optimization level
   (-Og / -Os). */
static void lcd_delay_us(uint32_t us)
{
	uint32_t n = us * (LCD_CPU_MHZ / 3u);

	if (n == 0) {
		return;
	}
	/* GCC 6 inline asm = divided syntax: "sub Rd, #imm" is the Thumb-1 SUBS */
	__asm__ volatile (
		"1:	sub	%0, #1		\n"
		"	bne	1b		\n"
		: "+l" (n)
		:
		: "cc");
}

static inline uint8_t nibble_bits(uint8_t nibble)
{
	return (uint8_t)((nibble & 0x0F) << LCD_PIN_D4);
}

/* Adds the RS setup state if RS differs from the previous output state. */
static uint8_t add_setup(uint8_t *s, uint8_t base, uint8_t first_data)
{
	if ((pcf8574_getoutput(LCD_PCF8574_DEVICEID) ^ base) & (BIT_RS | BIT_RW)) {
		s[0] = base | first_data;
		return 1;
	}
	return 0;
}

/* A single nibble (only in the 8-bit phase of init). */
static void lcd_write_nibble(uint8_t nibble, uint8_t rs)
{
	uint8_t s[3];
	uint8_t base = lcd_bl | rs;
	uint8_t d    = nibble_bits(nibble);
	uint8_t n    = add_setup(s, base, d);

	s[n++] = base | d | BIT_E;
	s[n++] = base | d;
	pcf8574_setoutputs(LCD_PCF8574_DEVICEID, s, n);
}

/* A whole byte in one frame (rs = 0 or BIT_RS). */
static void lcd_write_byte(uint8_t b, uint8_t rs)
{
	uint8_t s[5];
	uint8_t base = lcd_bl | rs;
	uint8_t hi   = nibble_bits(b >> 4);
	uint8_t lo   = nibble_bits(b);
	uint8_t n    = add_setup(s, base, hi);

	s[n++] = base | hi | BIT_E;
	s[n++] = base | hi;
	s[n++] = base | lo | BIT_E;
	s[n++] = base | lo;
	pcf8574_setoutputs(LCD_PCF8574_DEVICEID, s, n);
}


/* Internal HW instructions: they do not touch hw_addr, the caller maintains it. */
static void hd_command(uint8_t cmd)
{
	lcd_write_byte(cmd, 0);

	/* clear (0x01) and return home (0x02/0x03) are the only long instructions */
	if (cmd == HD_CLEAR || (cmd & 0xFE) == HD_HOME) {
		lcd_delay_us(LCD_CLEAR_US);
	}
}

static inline void hd_data(uint8_t data)
{
	lcd_write_byte(data, BIT_RS);
}

static void buffer_fill(char (*buf)[LCD_COLUMNS], char c)
{
	for (uint8_t y = 0; y < LCD_LINES; y++) {
		for (uint8_t x = 0; x < LCD_COLUMNS; x++) {
			buf[y][x] = c;
		}
	}
}


/* --------------------------------------------------------------------
   Raw HW functions: they bypass the buffer. The DDRAM address is marked
   unknown so that the next lcd_flush() sets it itself.
-------------------------------------------------------------------- */
void lcd_command(uint8_t cmd)
{
	hd_command(cmd);
	hw_addr = HW_ADDR_UNKNOWN;
}

void lcd_data(uint8_t data)
{
	hd_data(data);
	hw_addr = HW_ADDR_UNKNOWN;
}


/* Initialization per HD44780U datasheet figure 24 (4-bit). The three 8-bit
   function set nibbles resynchronize the controller whatever state it is in
   (e.g. MCU reset in the middle of a 4-bit byte without an LCD power cycle).
   delay_ms(n) waits > n-1 ms, so the minimum times are rounded up. */
void lcd_init(uint8_t dispAttr)
{
	pcf8574_init();

	/* PCF8574 power-on state is 0xFF (E and RW high): all low */
	pcf8574_setoutput(LCD_PCF8574_DEVICEID, lcd_bl);
	delay_ms(50);                   /* > 40 ms after Vcc rise */

	lcd_write_nibble(0x3, 0);
	delay_ms(6);                    /* > 4.1 ms */
	lcd_write_nibble(0x3, 0);
	lcd_delay_us(150);              /* > 100 us */
	lcd_write_nibble(0x3, 0);
	lcd_write_nibble(0x2, 0);       /* 4-bit mode */

	hd_command(HD_FUNC_4BIT | ((LCD_LINES > 1) ? HD_FUNC_2LINE : 0));
	hd_command(LCD_DISP_OFF);
	hd_command(HD_CLEAR);           /* the only HW clear: display and buffers empty */
	hd_command(HD_ENTRY_INC);
	hd_command(dispAttr);

	buffer_fill(frame, ' ');
	buffer_fill(shadow, ' ');
	shadow_valid = true;
	hw_addr = 0;                    /* clear sets the DDRAM address to 0 */
	cur_x = 0;
	cur_y = 0;
}

void lcd_led(uint8_t onoff)
{
	lcd_bl = onoff ? BIT_BL : 0;
	pcf8574_setoutput(LCD_PCF8574_DEVICEID,
	                  (uint8_t)((pcf8574_getoutput(LCD_PCF8574_DEVICEID) & ~BIT_BL) | lcd_bl));
}


/* --------------------------------------------------------------------
   Buffered functions: they write to frame[] only, not to the bus.
   The display is updated by lcd_flush().
-------------------------------------------------------------------- */

/* Clears the buffer. No HW clear (1.5-2.2 ms): flush sends only those
   spaces where the display shows something else. */
void lcd_clrscr(void)
{
	buffer_fill(frame, ' ');
	cur_x = 0;
	cur_y = 0;
}

void lcd_home(void)
{
	cur_x = 0;
	cur_y = 0;
}

/* Out-of-range coordinates are ignored. */
void lcd_gotoxy(uint8_t x, uint8_t y)
{
	if (x >= LCD_COLUMNS || y >= LCD_LINES) {
		return;
	}
	cur_x = x;
	cur_y = y;
}

/* '\n' moves to the start of the next line. When a line is full, the wrap
   happens only at the next character, so a full line + '\n' does not skip a
   line. The last line wraps to the first. */
void lcd_putc(char c)
{
	if (c == '\n') {
		lcd_gotoxy(0, (uint8_t)((cur_y + 1) % LCD_LINES));
		return;
	}
	if (cur_x >= LCD_COLUMNS) {
		lcd_gotoxy(0, (uint8_t)((cur_y + 1) % LCD_LINES));
	}
	frame[cur_y][cur_x] = c;
	cur_x++;
}

void lcd_puts(const char *s)
{
	while (*s) {
		lcd_putc(*s++);
	}
}

/* Marks the shadow as unreliable: the next lcd_flush() writes the whole
   display. Repairs wrong characters left on the display by a disturbance
   (ESD, I2C error) that a plain difference check would never notice. The
   DDRAM address is also marked unknown, as the disturbance may have moved it.
   Does not repair a controller reset (power loss): the controller is then
   in 8-bit mode. */
void lcd_invalidate(void)
{
	shadow_valid = false;
	hw_addr = HW_ADDR_UNKNOWN;
}

/* Sends only the changed characters (everything after an invalidate). The
   address command is sent only when the auto-incremented HD44780 address
   does not already point to the right place, so consecutive characters go
   with a single address command. */
void lcd_flush(void)
{
	for (uint8_t y = 0; y < LCD_LINES; y++) {
		for (uint8_t x = 0; x < LCD_COLUMNS; x++) {
			char c = frame[y][x];
			if (shadow_valid && c == shadow[y][x]) {
				continue;
			}
			uint8_t addr = (uint8_t)(line_start[y] + x);
			if (addr != hw_addr) {
				hd_command(HD_SET_DDRAM | addr);
			}
			hd_data((uint8_t)c);
			shadow[y][x] = c;
			hw_addr = (uint8_t)(addr + 1);  /* at the end of a line this does not match
			                                   the HW, but never hits a target address */
		}
	}
	shadow_valid = true;
}


/* --------------------------------------------------------------------
   CGRAM. Writing moves the address counter into CGRAM, so the DDRAM
   address is marked unknown. Characters 0..7 on the display change at once.
-------------------------------------------------------------------- */
void lcd_clear_CGRAM(void)
{
	hd_command(HD_SET_CGRAM);
	for (uint8_t i = 0; i < 64; i++) {
		hd_data(0);
	}
	hw_addr = HW_ADDR_UNKNOWN;
}

/* Defines character 0..7 (8 rows, 5 lowest bits). Returns the character code. */
uint8_t lcd_create_custom_char(uint8_t charnum, const uint8_t *values)
{
	charnum &= 0x07;
	hd_command(HD_SET_CGRAM | (uint8_t)(charnum << 3));
	for (uint8_t i = 0; i < 8; i++) {
		hd_data(values[i]);
	}
	hw_addr = HW_ADDR_UNKNOWN;
	return charnum;
}

#endif
