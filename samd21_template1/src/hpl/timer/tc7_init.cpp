/*
 * tc7_init.cpp
 *
 * Created: 23.9.2026 18.26.16
 *  Author: Kari63
 */ 


#include "timer7.h"
#include "tc7_init.h"
#include "samd21.h"


void tc7_init(void)
{
	TC7->COUNT16.CTRLA.bit.ENABLE = 0;
	while (TC7->COUNT16.STATUS.bit.SYNCBUSY) {}
	PM->APBCMASK.reg |= PM_APBCMASK_TC7;

	GCLK->CLKCTRL.reg =
	GCLK_CLKCTRL_ID_TC6_TC7 |
	GCLK_CLKCTRL_GEN_GCLK4   |
	GCLK_CLKCTRL_CLKEN;
	while (GCLK->STATUS.bit.SYNCBUSY) {}
	
	TC7->COUNT16.CTRLA.reg = TC_CTRLA_SWRST;
	while (TC7->COUNT16.CTRLA.bit.SWRST) {}

	TC7->COUNT16.CTRLA.reg =
	TC_CTRLA_MODE_COUNT16 |
	TC_CTRLA_WAVEGEN_NFRQ |            // capture use: MFRQ would use CC0 as TOP
	TC_CTRLA_PRESCALER_DIV1024;        // tick = 1024 / f_GCLK2, choose according to the press duration
	while (TC7->COUNT16.STATUS.bit.SYNCBUSY) {}

	TC7->COUNT16.CTRLC.reg =
	TC_CTRLC_CPTEN0 |                  // CC0 in capture mode (period)
	TC_CTRLC_CPTEN1;                   // CC1 in capture mode (pulse width)
	while (TC7->COUNT16.STATUS.bit.SYNCBUSY) {}

	TC7->COUNT16.EVCTRL.reg =
	TC_EVCTRL_TCEI |                   // accept EVSYS events
	TC_EVCTRL_EVACT_PPW;               // CC0 = period, CC1 = pulse width (event high = pulse)
	while (TC7->COUNT16.STATUS.bit.SYNCBUSY) {}

	TC7->COUNT16.INTENSET.reg = TC_INTENSET_MC1;   // interrupt at the end of the pulse (CC1 ready)

	NVIC_EnableIRQ(TC7_IRQn);

	TC7->COUNT16.CTRLA.reg |= TC_CTRLA_ENABLE;
	while (TC7->COUNT16.STATUS.bit.SYNCBUSY) {}
}
