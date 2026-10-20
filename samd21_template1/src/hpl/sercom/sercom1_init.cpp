/*
 * sercom1_init.cpp - SERCOM1 SPI master for the TFT (ST7735), TX only, 12 MHz
 * PA18 = PAD2 (MOSI), PA19 = PAD3 (SCK), function C. Formerly SERCOM3 on the
 * same pins (function D); moved so that SERCOM3 can take the EDBG virtual
 * COM port (PA22/PA23).
 */
#include "samd21.h"
#include "sercom_init.h"

void sercom1_init(void)
{
	SERCOM1->SPI.CTRLA.bit.ENABLE = 0;
	while (SERCOM1->SPI.SYNCBUSY.bit.ENABLE) {
	}

	PM->APBCMASK.reg |= PM_APBCMASK_SERCOM1;

	GCLK->CLKCTRL.reg = (uint16_t)(
		GCLK_CLKCTRL_ID(SERCOM1_GCLK_ID_CORE) |
		GCLK_CLKCTRL_GEN_GCLK0 |
		GCLK_CLKCTRL_CLKEN);
	while (GCLK->STATUS.bit.SYNCBUSY) {
	}

	SERCOM1->SPI.CTRLA.reg = SERCOM_SPI_CTRLA_SWRST;
	while (SERCOM1->SPI.SYNCBUSY.bit.SWRST) {
	}

	PORT->Group[0].PINCFG[18].reg = PORT_PINCFG_PMUXEN;
	PORT->Group[0].PMUX[9].bit.PMUXE = PORT_PMUX_PMUXE_C_Val;
	PORT->Group[0].PINCFG[19].reg = PORT_PINCFG_PMUXEN;
	PORT->Group[0].PMUX[9].bit.PMUXO = PORT_PMUX_PMUXO_C_Val;

	SERCOM1->SPI.CTRLA.reg =
		SERCOM_SPI_CTRLA_MODE(3) |
		SERCOM_SPI_CTRLA_DOPO(1) |
		SERCOM_SPI_CTRLA_DIPO(0);
	SERCOM1->SPI.CTRLB.reg = SERCOM_SPI_CTRLB_CHSIZE(0);
	while (SERCOM1->SPI.SYNCBUSY.bit.CTRLB) {
	}

	SERCOM1->SPI.BAUD.reg = 1;
	SERCOM1->SPI.CTRLA.bit.ENABLE = 1;
	SERCOM1->SPI.INTFLAG.reg = SERCOM_SPI_INTFLAG_TXC;
	while (SERCOM1->SPI.SYNCBUSY.bit.ENABLE) {
	}
}
