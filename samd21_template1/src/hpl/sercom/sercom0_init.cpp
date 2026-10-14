/*
 * sercom0_init.cpp
 *
 * Created: 24.8.2026 23.18.16
 *  Author: Kari63
 */ 
#include "project_config.h"



#include "samd21.h"
#include "sercom_init.h"
//#include "sercom0_init.h"

void sercom0_init(void)
{
	/* Enable SERCOM0 clock */
	PM->APBCMASK.reg |= PM_APBCMASK_SERCOM0;

	/* GCLK0 -> SERCOM0 core */
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(SERCOM0_GCLK_ID_CORE) |
	GCLK_CLKCTRL_GEN_GCLK0 |
	GCLK_CLKCTRL_CLKEN;
	while (GCLK->STATUS.bit.SYNCBUSY);

	/* PA08 = SDA (PAD0), PA09 = SCL (PAD1), MUX C */
	PORT->Group[0].PINCFG[8].bit.PMUXEN = 1;
	PORT->Group[0].PINCFG[9].bit.PMUXEN = 1;
	PORT->Group[0].PMUX[8 >> 1].reg = PORT_PMUX_PMUXE_C | PORT_PMUX_PMUXO_C;

	/* Software reset */
	SERCOM0->I2CM.CTRLA.reg = SERCOM_I2CM_CTRLA_SWRST;
	while (SERCOM0->I2CM.SYNCBUSY.bit.SWRST);

	/* I2C master mode */
	SERCOM0->I2CM.CTRLA.reg =
	SERCOM_I2CM_CTRLA_MODE_I2C_MASTER |
	SERCOM_I2CM_CTRLA_SPEED(0);

	/* Smart mode */
	SERCOM0->I2CM.CTRLB.reg = SERCOM_I2CM_CTRLB_SMEN;

	/* Approx. 400 kHz @ 48 MHz, ignoring SCL rise-time correction */
	SERCOM0->I2CM.BAUD.reg = 59;

	/* Enable */
	SERCOM0->I2CM.CTRLA.reg |= SERCOM_I2CM_CTRLA_ENABLE;
	while (SERCOM0->I2CM.SYNCBUSY.bit.ENABLE);

	/* Clear any bus errors picked up from the lines during start-up */
	SERCOM0->I2CM.STATUS.reg = SERCOM_I2CM_STATUS_BUSERR | SERCOM_I2CM_STATUS_ARBLOST;

	/* Force the bus state to IDLE (1) */
	SERCOM0->I2CM.STATUS.reg = SERCOM_I2CM_STATUS_BUSSTATE(1);
	
	/* Wait until the hardware reports the bus as IDLE */
	while ((SERCOM0->I2CM.STATUS.reg & SERCOM_I2CM_STATUS_BUSSTATE_Msk) != SERCOM_I2CM_STATUS_BUSSTATE(1));
}

