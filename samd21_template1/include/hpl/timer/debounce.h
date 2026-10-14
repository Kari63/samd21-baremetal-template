

/*
 * debounce.h
 *
 * Created: 7.8.2026 18.34.32
 *  Author: Kari63
 */

#pragma once
#include <stdint.h>
#include "gpio_simplex.h"






// Debounce state of one button
typedef struct {
    GpioPin pin;          // button pin
    uint8_t stable_state; // last confirmed state
    uint8_t last_raw;     // last raw state
    uint32_t last_change; // time of the last change (systick_ms_laskuri)
    uint32_t debounce_ms; // debounce time in milliseconds
} Debounce;

// Initialization
void debounce_init(Debounce *d, GpioPin pin, uint32_t debounce_ms);

// Returns the confirmed state (0 = pressed, 1 = released)
uint8_t debounce_read(Debounce *d);

// Returns the press edge (1 once when the button is pressed)
uint8_t debounce_edge(Debounce *d);




