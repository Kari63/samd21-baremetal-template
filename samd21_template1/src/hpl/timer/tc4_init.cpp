/*
 * tc4_init.cpp
 *
 * Created: 2.9.2026 16.05.32
 *  Author: Kari63
 */ 
    


#include "tc4_init.h"
#include "timer4.h"
#include "samd21.h"


void tc4_init(void)
{	 TC4->COUNT16.CTRLA.bit.ENABLE = 0;
	while (TC4->COUNT16.STATUS.bit.SYNCBUSY) {}
	PM->APBCMASK.reg |= PM_APBCMASK_TC4;

	GCLK->CLKCTRL.reg =
	GCLK_CLKCTRL_ID_TC4_TC5 |
	GCLK_CLKCTRL_GEN_GCLK4  |
	GCLK_CLKCTRL_CLKEN;
	while (GCLK->STATUS.bit.SYNCBUSY) {}
	
	TC4->COUNT16.CTRLA.reg = TC_CTRLA_SWRST;
	while (TC4->COUNT16.CTRLA.bit.SWRST) {}

	TC4->COUNT16.CTRLA.reg =
	TC_CTRLA_MODE_COUNT16 |
	TC_CTRLA_WAVEGEN_MFRQ|          //  timer use
	TC_CTRLA_PRESCALER_DIV1024;
	while (TC4->COUNT16.STATUS.bit.SYNCBUSY) {}
		
	TC4->COUNT16.CC[0].reg = next_top4;	 // TOP

	TC4->COUNT16.EVCTRL.reg |= TC_EVCTRL_MCEO0;

	TC4->COUNT16.INTENSET.reg = TC_INTENSET_MC0 ;

	NVIC_EnableIRQ(TC4_IRQn);

	TC4->COUNT16.CTRLA.reg |= TC_CTRLA_ENABLE;
	while (TC4->COUNT16.STATUS.bit.SYNCBUSY) {}
}
