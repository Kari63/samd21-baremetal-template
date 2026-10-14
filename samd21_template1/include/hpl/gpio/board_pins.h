/*
 * board_pins.h
 * gpio_led.h
 *
 * Created: 11.9.2026 2.17.39
 *  Author: Kari63
 */ 


#pragma once

#include "gpio_simplex.h"

extern const GpioPin LED;
extern const GpioPin SW0;
// Initializes the fixed board pins (e.g. LED)
void board_gpio_init(void);


