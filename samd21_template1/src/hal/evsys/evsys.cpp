/*
 * evsys.cpp
 *
 * Created: 18.9.2026 13.16.33
 *  Author: Kari63
 */
#include "project_config.h"

#if USE_EVSYS

#include "samd21.h"
#include "evsys.h"
#include <stddef.h>


#define EVSYS_EIC_TC7_CHANNEL      0     //1 = TC4, 3 = SW trigger)
#define EVSYS_TC4_CHANNEL          1
#define EVSYS_TC3_ADC_CHANNEL      2
#define EVSYS_SW_TRIGGER_CHANNEL   3
#define EVSYS_ID_GEN_NONE    0 



void evsys_route_eic_to_tc7(uint8_t extint_line)
{
	evsys_channel_configure(EVSYS_EIC_TC7_CHANNEL,EVSYS_ID_GEN_EIC_EXTINT_0 + extint_line,
	EVSYS_CHANNEL_PATH_ASYNCHRONOUS );
	evsys_user_configure(EVSYS_ID_USER_TC7_EVU, EVSYS_EIC_TC7_CHANNEL);   // the +1 offset is handled inside
}



void evsys_sw_trigger(void)
{
	EVSYS->CHANNEL.reg =
	EVSYS_CHANNEL_CHANNEL(EVSYS_SW_TRIGGER_CHANNEL) |
	EVSYS_CHANNEL_SWEVT;
}

void evsys_route_sw_trig_to_tc6(void)
{
	evsys_channel_configure(EVSYS_SW_TRIGGER_CHANNEL, EVSYS_ID_GEN_NONE,
	EVSYS_CHANNEL_PATH_SYNCHRONOUS | EVSYS_CHANNEL_EDGSEL_RISING_EDGE);
	
	evsys_user_configure(EVSYS_ID_USER_TC6_EVU, EVSYS_SW_TRIGGER_CHANNEL);
	 
}


void evsys_route_tc4_to_dmac0_and_tc5(void)
{
	evsys_channel_configure(EVSYS_TC4_CHANNEL, EVSYS_ID_GEN_TC4_MCX_0, EVSYS_CHANNEL_PATH_ASYNCHRONOUS);
	
	evsys_user_configure(EVSYS_ID_USER_DMAC_CH_0, EVSYS_TC4_CHANNEL);
	evsys_user_configure(EVSYS_ID_USER_TC5_EVU, EVSYS_TC4_CHANNEL);

}

void evsys_route_tc3_to_adc(void)
{
	// The TC3 MC0 event travels on this channel to the ADC START user.
	evsys_channel_configure(EVSYS_TC3_ADC_CHANNEL, EVSYS_ID_GEN_TC3_MCX_0,
		EVSYS_CHANNEL_PATH_ASYNCHRONOUS);
	evsys_user_configure(EVSYS_ID_USER_ADC_START, EVSYS_TC3_ADC_CHANNEL);
}

// One channel, one generator - maps directly to the CHANNEL register
void evsys_channel_configure(uint8_t channel, uint16_t generator_id, uint32_t path)
{
	EVSYS->CHANNEL.reg =
	EVSYS_CHANNEL_CHANNEL(channel) |
	EVSYS_CHANNEL_EVGEN(generator_id) |
	path;
}

// One user connected to a channel - maps directly to the USER register
// NOTE: the channel+1 offset is applied HERE, not in the caller
void evsys_user_configure(uint16_t user_id, uint8_t channel)
{
	EVSYS->USER.reg =
	EVSYS_USER_USER(user_id) |
	EVSYS_USER_CHANNEL(channel + 1);   // the Microchip +1 quirk is hidden here
}


void evsys_init(void)
{
	PM->APBCMASK.reg |= PM_APBCMASK_EVSYS;
	
	// Synchronous software events on channel 3 need its generic clock.
	GCLK->CLKCTRL.reg =
	GCLK_CLKCTRL_ID_EVSYS_3 |
	GCLK_CLKCTRL_GEN_GCLK0 |
	GCLK_CLKCTRL_CLKEN;
	while (GCLK->STATUS.bit.SYNCBUSY) {}
		
	GCLK->CLKCTRL.reg =
	GCLK_CLKCTRL_ID_EVSYS_0 |
	GCLK_CLKCTRL_GEN_GCLK0 |
	GCLK_CLKCTRL_CLKEN;
	while (GCLK->STATUS.bit.SYNCBUSY) {}

	EVSYS->CTRL.reg = EVSYS_CTRL_SWRST;
	while (EVSYS->CTRL.reg & EVSYS_CTRL_SWRST){}
	EVSYS->CTRL.reg = EVSYS_CTRL_GCLKREQ;

	evsys_route_tc3_to_adc();
}


void evsys_route_init(void)
{
		   /* CH1: TC4 -> DMAC0 + TC5 */
	   EVSYS->CHANNEL.reg =
	   EVSYS_CHANNEL_CHANNEL(EVSYS_TC4_CHANNEL) |
	   EVSYS_CHANNEL_EVGEN(EVSYS_ID_GEN_TC4_MCX_0) |
	   EVSYS_CHANNEL_PATH_ASYNCHRONOUS;
	   while (EVSYS->CHSTATUS.bit.CHBUSY1) {}

	   EVSYS->USER.reg = EVSYS_USER_USER(EVSYS_ID_USER_DMAC_CH_0) | EVSYS_USER_CHANNEL(2);
	   while (!EVSYS->CHSTATUS.bit.USRRDY1) {}
	   EVSYS->USER.reg = EVSYS_USER_USER(EVSYS_ID_USER_TC5_EVU) | EVSYS_USER_CHANNEL(2);
	   while (!EVSYS->CHSTATUS.bit.USRRDY1) {}


		/* CH3: SW Trigger -> TC6; leave this channel selected for SWEVT. */
		EVSYS->CHANNEL.reg =
		EVSYS_CHANNEL_CHANNEL(EVSYS_SW_TRIGGER_CHANNEL) |
		EVSYS_CHANNEL_EVGEN(EVSYS_ID_GEN_NONE) |
		EVSYS_CHANNEL_PATH_SYNCHRONOUS |
		EVSYS_CHANNEL_EDGSEL_RISING_EDGE;
		while (EVSYS->CHSTATUS.bit.CHBUSY3) {}

		EVSYS->USER.reg = EVSYS_USER_USER(EVSYS_ID_USER_TC6_EVU) | EVSYS_USER_CHANNEL(4);
		while (!EVSYS->CHSTATUS.bit.USRRDY3) {}
		EVSYS->INTENSET.reg = EVSYS_INTENSET_EVD3;
	
	
}

#endif /* USE_EVSYS */

