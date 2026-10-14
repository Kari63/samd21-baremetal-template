/*
 * timer6.cpp
 *
 * Created: 23.9.2026 18.53.03
 *  Author: Kari63
 */ 

#include "project_config.h"

#if USE_TC6

#include "timer6.h"
#include "samd21.h"

// Tick time in microseconds = PRESCALER / 8 (8 MHz GCLK2)
// DIV256 -> 32, DIV64 -> 8, DIV16 -> 2, DIV8 -> 1
#define TC6_TICK_US  1UL 

static uint16_t tc6_cc0;
static uint32_t tc6_mc0_count;
static uint32_t tc6_ovf_count;

static uint16_t tc6_delta;
static uint32_t tc6_delta_ms;

static volatile uint16_t tc6_min;
static volatile uint16_t tc6_max;

static uint16_t tc6_prev;                    // NOT volatile
static volatile uint8_t tc6_ignore_next_delta;


// Weak callback (the application overrides it)
__attribute__((weak)) void timer6_onMatch() {}

void tc6_ignore_next_interval(void)
{
	tc6_ignore_next_delta = 1;
}

extern "C" void TC6_Handler(void)
{
	uint8_t flags = TC6->COUNT16.INTFLAG.reg;

	if (flags & TC_INTFLAG_MC0)
	{
		tc6_cc0 = TC6->COUNT16.CC[0].reg;

		tc6_delta = (uint16_t)(tc6_cc0 - tc6_prev);   // wrap-safe
		tc6_prev  = tc6_cc0;

		uint32_t delta_us = (uint32_t)tc6_delta * TC6_TICK_US;
		tc6_delta_ms = (delta_us + 500UL) / 1000UL;

		uint8_t ignore_delta = tc6_ignore_next_delta;
		tc6_ignore_next_delta = 0;

		// Min/max: the first delta is invalid (tc6_prev uninitialized), skip it
		if (tc6_mc0_count > 0 && !ignore_delta)
		{
			if (tc6_delta < tc6_min) tc6_min = tc6_delta;
			if (tc6_delta > tc6_max) tc6_max = tc6_delta;
		}

		tc6_mc0_count++;
	}

	if (flags & TC_INTFLAG_OVF)
	{
		tc6_ovf_count++;
	}

	TC6->COUNT16.INTFLAG.reg = flags;
}


// Reads the window min/max and starts a new window.
// Interrupts are disabled briefly so the ISR cannot run between the read and the reset.
void tc6_minmax_take(uint16_t &mn, uint16_t &mx)
{
	uint32_t primask = __get_PRIMASK();
	__disable_irq();
	mn = tc6_min;
	mx = tc6_max;
	tc6_min = 0xFFFF;
	tc6_max = 0;
	__set_PRIMASK(primask);
}

#endif /* USE_TC6 */