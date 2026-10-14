/*
 * eic_init.cpp
 *
 * Created: 30.10.2026 8.15.05
 *  Author: Kari63
 */ 


#include "eic_init.h"
#include "samd21.h"

void eic_enable(void)
{
	EIC->CTRL.reg = EIC_CTRL_ENABLE;
	while (EIC->STATUS.reg & EIC_STATUS_SYNCBUSY) {}
}

void eic_disable(void)
{
	EIC->CTRL.reg = 0;
	while (EIC->STATUS.reg & EIC_STATUS_SYNCBUSY) {}
}

void eic_init(void)
{
	// 1. Enable EIC APB clock
	PM->APBAMASK.reg |= PM_APBAMASK_EIC;

	// 2. EIC clock from GCLK0
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID_EIC |
	                    GCLK_CLKCTRL_GEN_GCLK0 |
	                    GCLK_CLKCTRL_CLKEN;
	while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY) {}

	// 3. Reset EIC
	EIC->CTRL.reg = EIC_CTRL_SWRST;
	while ((EIC->CTRL.reg & EIC_CTRL_SWRST) ||
	       (EIC->STATUS.reg & EIC_STATUS_SYNCBUSY)) {}

	// 4. Enable EIC
	eic_enable();

	// 5. NVIC
	NVIC_EnableIRQ(EIC_IRQn);
}
