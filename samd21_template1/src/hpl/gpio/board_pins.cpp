/*
 * board_pins.cpp
 *
 * Created: 29.9.2026
 *  Author: Kari63
 */

#include "board_pins.h"
#include "gpio_pins.h"

// SAMD21 Xplained Pro: on-board LED (fixed HW pin)
const GpioPin LED = PB(30);
const GpioPin SW0 = PA(15);

void board_gpio_init(void)
{
	gpio_init_output(LED);
    gpio_init_input(SW0);
    gpio_init_input_pullup(SW0);
}
