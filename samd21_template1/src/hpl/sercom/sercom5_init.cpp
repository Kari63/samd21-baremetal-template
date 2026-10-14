#include "samd21.h"
#include "sercom_init.h"


void sercom5_init(void)
{
	
	SERCOM5->SPI.CTRLA.bit.ENABLE = 0;
	while (SERCOM5->SPI.SYNCBUSY.bit.ENABLE);
	
	PM->APBCMASK.reg |= PM_APBCMASK_SERCOM5;
	
	GCLK->CLKCTRL.reg = (uint16_t)(
	GCLK_CLKCTRL_ID(SERCOM5_GCLK_ID_CORE) |
	GCLK_CLKCTRL_GEN_GCLK0 |
	GCLK_CLKCTRL_CLKEN
	);
	while (GCLK->STATUS.bit.SYNCBUSY); // wait for GCLK synchronization

	SERCOM5->SPI.CTRLA.reg = SERCOM_SPI_CTRLA_SWRST;
	while (SERCOM5->SPI.SYNCBUSY.bit.SWRST);
		
	// PB22 = pad 2 (MOSI) -> function D (value 3)
	PORT->Group[1].PINCFG[22].reg = PORT_PINCFG_PMUXEN;
	// Clear the old value first and set function D (3)
	PORT->Group[1].PMUX[22 >> 1].reg &= ~PORT_PMUX_PMUXE_Msk;
	PORT->Group[1].PMUX[22 >> 1].reg |= PORT_PMUX_PMUXE_D;
	
	// PB16 = pad 0 (MISO) -> function C (value 2)
	PORT->Group[1].PINCFG[16].reg = PORT_PINCFG_PMUXEN | PORT_PINCFG_INEN;
	PORT->Group[1].PMUX[16 >> 1].reg |= PORT_PMUX_PMUXE_C;
	

	// PB23 = pad 3 (SCK) -> function D (value 3)
	PORT->Group[1].PINCFG[23].reg = PORT_PINCFG_PMUXEN;
	PORT->Group[1].PMUX[23 >> 1].reg &= ~PORT_PMUX_PMUXO_Msk;
	PORT->Group[1].PMUX[23 >> 1].reg |= PORT_PMUX_PMUXO_D;
	
	// PA13 = CS -> plain GPIO output (PMUXEN = 0)
	PORT->Group[0].PINCFG[13].reg = 0;
	PORT->Group[0].DIRSET.reg = (1u << 13);
	PORT->Group[0].OUTSET.reg = (1u << 13); // CS Idle = High

	/* --- SERCOM5 SPI REGISTER INIT --- */

	SERCOM5->SPI.CTRLA.reg =
	SERCOM_SPI_CTRLA_MODE(3) | // SPI Master
	SERCOM_SPI_CTRLA_DOPO(1) | // Pad 2 = MOSI, Pad 3 = SCK
	SERCOM_SPI_CTRLA_DIPO(0) ;  // Pad 0 = MISO
	
	

	SERCOM5->SPI.CTRLB.reg =
	SERCOM_SPI_CTRLB_RXEN |
	SERCOM_SPI_CTRLB_CHSIZE(0); // Full-duplex SPI for external flash

	while (SERCOM5->SPI.SYNCBUSY.bit.CTRLB);

	// BAUD = 1 (12 MHz)
	SERCOM5->SPI.BAUD.reg = 1;

	// 5. Enable the SERCOM5 SPI
	SERCOM5->SPI.CTRLA.bit.ENABLE = 1;

	// Make sure no pending interrupts are left from old TXC flags or enables
	// before the SERCOM is used.
	SERCOM5->SPI.INTENCLR.reg = SERCOM_SPI_INTENCLR_TXC;
	SERCOM5->SPI.INTFLAG.reg = SERCOM_SPI_INTFLAG_TXC;

	//NVIC_EnableIRQ(SERCOM5_IRQn);
	while (SERCOM5->SPI.SYNCBUSY.bit.ENABLE);
	
	
}
