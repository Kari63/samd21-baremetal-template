#include "project_config.h"

#if USE_TFT

#include "spi3.h"
#include "samd21.h"

void spi3_transmit(uint8_t data)
{
	while (!SERCOM3->SPI.INTFLAG.bit.DRE) {
	}
	SERCOM3->SPI.DATA.reg = data;
	while (!SERCOM3->SPI.INTFLAG.bit.TXC) {
	}
}

void spi3_transmit_stream(uint8_t data)
{
	while (!SERCOM3->SPI.INTFLAG.bit.DRE) {
	}
	SERCOM3->SPI.DATA.reg = data;
}

#endif
