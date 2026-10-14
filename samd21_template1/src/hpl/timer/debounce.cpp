/*
 * debounce.cpp
 *
 * Created: 7.8.2026 18.34.57
 *  Author: Kari63
 */ 
#include "debounce.h"
#include "systick.h"	// systick_ms_laskuri: 1 ms SysTick counter

void debounce_init(Debounce *d, GpioPin pin, uint32_t debounce_ms)
{
	d->pin          = pin;
	d->debounce_ms  = debounce_ms;
	d->stable_state = 1;          // pull-up default
	d->last_raw     = 1;
	d->last_change  = systick_ms_laskuri;
}

uint8_t debounce_read(Debounce *d)
{
	uint8_t raw = gpio_read(d->pin);

	// Raw state changed -> start the debounce timer
	if (raw != d->last_raw)
	{
		d->last_raw = raw;
		d->last_change = systick_ms_laskuri;
	}

	// State stable for debounce_ms -> accept the change
	if ((systick_ms_laskuri - d->last_change) >= d->debounce_ms)
	{
		d->stable_state = raw;
	}

	return d->stable_state;
}

uint8_t debounce_edge(Debounce *d)
{
	uint8_t prev = d->stable_state;
	uint8_t now  = debounce_read(d);

	// pull-up -> press edge = 1 -> 0
	return (prev == 1 && now == 0);
}
