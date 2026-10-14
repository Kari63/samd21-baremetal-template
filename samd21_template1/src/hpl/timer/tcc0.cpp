#include "samd21.h"
#include "timer_init.h"

void tcc0_init(void)
{
	TCC0->CTRLA.bit.ENABLE = 0;
	while (TCC0->SYNCBUSY.bit.ENABLE);

	PM->APBCMASK.reg |= PM_APBCMASK_TCC0;

	GCLK->CLKCTRL.reg =
	GCLK_CLKCTRL_ID_TCC0_TCC1 |
	GCLK_CLKCTRL_GEN_GCLK4 |
	GCLK_CLKCTRL_CLKEN;
	while (GCLK->STATUS.bit.SYNCBUSY);

	TCC0->CTRLA.reg = TCC_CTRLA_SWRST;
	while (TCC0->CTRLA.bit.SWRST);

	TCC0->CTRLA.reg = TCC_CTRLA_PRESCALER_DIV1;
	TCC0->WAVE.reg = TCC_WAVE_WAVEGEN_NPWM;
	while (TCC0->SYNCBUSY.reg) ;


	

	
    PORT->Group[0].PINCFG[4].bit.PMUXEN = 1;
	PORT->Group[0].PINCFG[5].bit.PMUXEN = 1;
	
	PORT->Group[0].PMUX[2].bit.PMUXE = PORT_PMUX_PMUXE_E_Val;
	PORT->Group[0].PMUX[2].bit.PMUXO = PORT_PMUX_PMUXO_E_Val;
	
	

	
	TCC0->PER.reg = 2400;
	TCC0->CC[0].reg = 1200;
	TCC0->CC[1].reg = 1200;


	
	
	while (TCC0->SYNCBUSY.reg);

	TCC0->EVCTRL.reg = 0;
	TCC0->INTENCLR.reg = TCC_INTENCLR_MC0;
	TCC0->INTFLAG.reg = TCC_INTFLAG_MC0;

	TCC0->CTRLA.bit.ENABLE = 1;
	while (TCC0->SYNCBUSY.bit.ENABLE);
}
