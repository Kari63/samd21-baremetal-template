/*
 * tc6_init.cpp — CAPTURE (main loop time measurement with a SW trigger)
 * Same model as TC5, applied to TC6.
 */

#include "samd21.h"



void tc6_init(void)
{
    TC6->COUNT16.CTRLA.bit.ENABLE = 0;
    while (TC6->COUNT16.STATUS.bit.SYNCBUSY) {}

    PM->APBCMASK.reg |= PM_APBCMASK_TC6;

    GCLK->CLKCTRL.reg =
          GCLK_CLKCTRL_ID_TC6_TC7
        | GCLK_CLKCTRL_GEN_GCLK4
        | GCLK_CLKCTRL_CLKEN;
    while (GCLK->STATUS.bit.SYNCBUSY) {}

    TC6->COUNT16.CTRLA.reg = TC_CTRLA_SWRST;
    while (TC6->COUNT16.CTRLA.bit.SWRST) {}

    // Free running, no EVACT_COUNT - counts the clock, not events
    TC6->COUNT16.CTRLA.reg =
          TC_CTRLA_MODE_COUNT16
        | TC_CTRLA_PRESCALER_DIV8;   // same principle as TC5, adjust if needed
    while (TC6->COUNT16.STATUS.bit.SYNCBUSY) {}

    // CC0 as capture register (not compare), no CPTEN1/PPW
    TC6->COUNT16.CTRLC.reg = TC_CTRLC_CPTEN0;
    while (TC6->COUNT16.STATUS.bit.SYNCBUSY) {}

    // Plain capture from the EVSYS event (no EVACT_PPW)
    TC6->COUNT16.EVCTRL.reg = TC_EVCTRL_TCEI;
    while (TC6->COUNT16.STATUS.bit.SYNCBUSY) {}

    TC6->COUNT16.INTENCLR.reg = 0xFF;
    TC6->COUNT16.INTENSET.reg =
          TC_INTENSET_MC0
        | TC_INTENSET_OVF;           // OVF for diagnostics, not required

    NVIC_ClearPendingIRQ(TC6_IRQn);
    NVIC_EnableIRQ(TC6_IRQn);         // note: TC6_IRQn, not TC4/TC5

    TC6->COUNT16.COUNT.reg = 0;
    while (TC6->COUNT16.STATUS.bit.SYNCBUSY) {}

    TC6->COUNT16.CTRLA.reg |= TC_CTRLA_ENABLE;
    while (TC6->COUNT16.STATUS.bit.SYNCBUSY) {}
}



		
		
		
	
	
	


	
		
		
		
		
		
		
	
	
	

