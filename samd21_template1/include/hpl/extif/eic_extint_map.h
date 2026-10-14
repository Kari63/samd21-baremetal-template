#pragma once

#include <stdint.h>

/*
 * Pin -> EXTINT line, SAMD21J (64-pin).
 * Source: SAM D21 datasheet DS40001882D, Table 7-1 (I/O Multiplexing), column A = EIC.
 *
 * Index = pin number within the port (PA00..PA31 / PB00..PB31).
 * -1 = the pin has no EXTINT line (NMI, or the pin does not exist in the J package).
 */

#define EXTINT_LINE_NONE  (-1)

static const int8_t k_extint_porta[32] = {
	/* PA00..PA07 */  0,  1,  2,  3,  4,  5,  6,  7,
	/* PA08..PA15 */ -1,  9, 10, 11, 12, 13, 14, 15,   /* PA08 = NMI */
	/* PA16..PA23 */  0,  1,  2,  3,  4,  5,  6,  7,
	/* PA24..PA31 */ 12, 13, -1, 15,  8, -1, 10, 11    /* PA26, PA29 do not exist */
};

static const int8_t k_extint_portb[32] = {
	/* PB00..PB07 */  0,  1,  2,  3,  4,  5,  6,  7,
	/* PB08..PB15 */  8,  9, 10, 11, 12, 13, 14, 15,
	/* PB16..PB23 */  0,  1, -1, -1, -1, -1,  6,  7,   /* PB18..PB21 do not exist */
	/* PB24..PB31 */ -1, -1, -1, -1, -1, -1, 14, 15    /* PB24..PB29 do not exist */
};

/* port: 0 = PORTA, 1 = PORTB (same numbers as PORTA/PORTB in gpio_pins.h) */
static inline int8_t extint_line(uint8_t port, uint8_t pin)
{
	if (pin >= 32) return EXTINT_LINE_NONE;
	return (port == 0) ? k_extint_porta[pin] : k_extint_portb[pin];
}
