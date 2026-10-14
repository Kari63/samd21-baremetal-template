/*
 * tc5_init.cpp
 *
 * Created: 5.9.2026 19.05.02
 *  Author: Kari63
 */ 
    

#include "tc5_init.h"
#include "samd21.h"


void tc5_init(void)
{
	TC5->COUNT16.CTRLA.bit.ENABLE = 0;
	while (TC5->COUNT16.STATUS.bit.SYNCBUSY) {}
	PM->APBCMASK.reg |= PM_APBCMASK_TC5;

	GCLK->CLKCTRL.reg =
	GCLK_CLKCTRL_ID_TC4_TC5 |
	GCLK_CLKCTRL_GEN_GCLK4   |
	GCLK_CLKCTRL_CLKEN;
	while (GCLK->STATUS.bit.SYNCBUSY) {}
	
	TC5->COUNT16.CTRLA.reg = TC_CTRLA_SWRST;
	while (TC5->COUNT16.CTRLA.bit.SWRST) {}

	TC5->COUNT16.CTRLA.reg =
	TC_CTRLA_MODE_COUNT16 |
	TC_CTRLA_WAVEGEN_MFRQ ;        // MFRQ: COUNT is cleared automatically on a CC0 match
	//TC_CTRLA_PRESCALER_DIV1024;
	while (TC5->COUNT16.STATUS.bit.SYNCBUSY) {}
	
	TC5->COUNT16.CC[0].reg = 0;	 // match on the first incoming TC4 event

	//TC5->COUNT16.EVCTRL.reg |= TC_EVCTRL_MCEO0;  //normal timer
	
	TC5->COUNT16.EVCTRL.reg =
	TC_EVCTRL_TCEI |                    // accept EVSYS events (instead of MCEO0)
	TC_EVCTRL_EVACT_COUNT;              // count every incoming event
	while (TC5->COUNT16.STATUS.bit.SYNCBUSY) {}

	TC5->COUNT16.INTENSET.reg = TC_INTENSET_MC0 ;

	NVIC_EnableIRQ(TC5_IRQn);

	TC5->COUNT16.CTRLA.reg |= TC_CTRLA_ENABLE;
	while (TC5->COUNT16.STATUS.bit.SYNCBUSY) {}
}

	
		
		
		
		
		
		
	
	
	

