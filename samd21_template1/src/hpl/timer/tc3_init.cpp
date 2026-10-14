

#include "tc3_init.h"
#include "samd21.h"


void tc3_init(void)
{	 TC3->COUNT16.CTRLA.bit.ENABLE = 0;
	 while (TC3->COUNT16.STATUS.bit.SYNCBUSY) {}
    PM->APBCMASK.reg |= PM_APBCMASK_TC3;

	GCLK->CLKCTRL.reg =
	GCLK_CLKCTRL_ID_TCC2_TC3 |
	GCLK_CLKCTRL_GEN_GCLK4   |
	GCLK_CLKCTRL_CLKEN;
	while (GCLK->STATUS.bit.SYNCBUSY) {}	
		
    TC3->COUNT16.CTRLA.reg = TC_CTRLA_SWRST;
	while (TC3->COUNT16.CTRLA.bit.SWRST) {} 

    TC3->COUNT16.CTRLA.reg =
    TC_CTRLA_MODE_COUNT16 |
    TC_CTRLA_WAVEGEN_MFRQ|  // no duty cycle (MPWM for duty)
    TC_CTRLA_PRESCALER_DIV1;      
    while (TC3->COUNT16.STATUS.bit.SYNCBUSY) {}
	

	
//	PORT->Group[0].PINCFG[19].bit.PMUXEN = 1;
//  PORT->Group[0].PMUX[19 >> 1].bit.PMUXO = PORT_PMUX_PMUXO_E_Val;	
    TC3->COUNT16.CC[0].reg =   7990; 
	TC3->COUNT16.CC[1].reg =  0 ;   
	   
    while (TC3->COUNT16.STATUS.bit.SYNCBUSY) {}

    TC3->COUNT16.EVCTRL.reg = TC_EVCTRL_MCEO0;
    while (TC3->COUNT16.STATUS.bit.SYNCBUSY) {}
		
   
    
    TC3->COUNT16.INTENCLR.reg = TC_INTENCLR_MC0;
    TC3->COUNT16.INTFLAG.reg = TC_INTFLAG_MC0;
    NVIC_DisableIRQ(TC3_IRQn);
    NVIC_ClearPendingIRQ(TC3_IRQn);

    TC3->COUNT16.CTRLA.reg |= TC_CTRLA_ENABLE;
    while (TC3->COUNT16.STATUS.bit.SYNCBUSY) {}
}
