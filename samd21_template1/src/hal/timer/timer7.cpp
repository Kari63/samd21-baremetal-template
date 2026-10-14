/*
 * timer7.cpp
 *
 * Created: 30.9.2026 03.35.16
 *  Author: Kari63
 */ 



#include "project_config.h"

#if USE_TC7

#include "timer7.h"
#include "samd21.h"
#include <stddef.h>






// PPW measurement results in ticks (tick = 1024 / f_GCLK2 = 128 us)
static uint16_t tc7_pulse_ticks  = 0;   // CC1: pulse width (LOW time)
static uint16_t tc7_period_ticks = 0;   // CC0: period (rising edge -> next rising edge)

volatile uint32_t  tc7pw = 0;
volatile uint32_t  f_x100 = 0;


// CC register addresses for READREQ (0x18 and 0x1A)
#define TC7_CC0_ADDR  (offsetof(TcCount16, CC))
#define TC7_CC1_ADDR  (offsetof(TcCount16, CC) + sizeof(uint16_t))

// Reading a CC register needs READREQ (the register is in another clock domain)
static uint16_t tc7_read_cc(uint32_t addr, uint32_t idx)
{
	TC7->COUNT16.READREQ.reg = TC_READREQ_RREQ | TC_READREQ_ADDR(addr);
	while (TC7->COUNT16.STATUS.bit.SYNCBUSY) {}
	return TC7->COUNT16.CC[idx].reg;
}

// Weak callback (the application overrides it)
// Called when the pulse width has been measured (button released), results: tc7_pulse_ticks, tc7_period_ticks
__attribute__((weak)) void timer7_onMatch() {}

void TC7_Handler(void)
{
	// Only the enabled sources; all handled flags are cleared at the end
	const uint8_t active = TC7->COUNT16.INTFLAG.reg & TC7->COUNT16.INTENSET.reg;

	if (active & TC_INTFLAG_MC1)
	{
		tc7_period_ticks = tc7_read_cc(TC7_CC0_ADDR, 0);   // the first measurement is garbage (no previous edge)
		tc7_pulse_ticks  = tc7_read_cc(TC7_CC1_ADDR, 1);
		
		tc7pw = ((uint32_t)tc7_pulse_ticks * 128u)/ 1000UL;
		f_x100 = ((uint32_t)781250u / tc7_period_ticks)/100UL;
		
		

		timer7_onMatch();
	}

	TC7->COUNT16.INTFLAG.reg = active | TC_INTFLAG_MC0 | TC_INTFLAG_ERR;
}

#endif /* USE_TC7 */
