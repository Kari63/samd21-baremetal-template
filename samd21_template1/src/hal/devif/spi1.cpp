#include "project_config.h"

#if USE_TFT

#include "spi1.h"
#include "samd21.h"

void spi1_transmit(uint8_t data)
{
	while (!SERCOM1->SPI.INTFLAG.bit.DRE) {
	}
	SERCOM1->SPI.DATA.reg = data;
	while (!SERCOM1->SPI.INTFLAG.bit.TXC) {
	}
}

void spi1_transmit_stream(uint8_t data)
{
	while (!SERCOM1->SPI.INTFLAG.bit.DRE) {
	}
	SERCOM1->SPI.DATA.reg = data;
}

#endif
