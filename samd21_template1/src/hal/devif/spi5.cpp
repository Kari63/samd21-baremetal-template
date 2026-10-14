/*
 * spi5.cpp
 *
 * Created: 21.8.2026 15.03.27
 *  Author: Kari63
 */ 
#include "project_config.h"

#if USE_SPI

#include "spi5.h"
#include <stdint.h>
#include "samd21.h"


void spi5_flash_cs_low()
{
	
	PORT->Group[0].OUTCLR.reg = (1 << 13);
}

void spi5_flash_cs_high()
{
	
	PORT->Group[0].OUTSET.reg = (1 << 13);
}

void spi5_drain_rx(void)
{
	while (SERCOM5->SPI.INTFLAG.bit.RXC) {
		(void)SERCOM5->SPI.DATA.reg;
	}
}

void spi5_transmit_stream(uint8_t data)
{
	while (!SERCOM5->SPI.INTFLAG.bit.DRE) {
		spi5_drain_rx();
	}
	spi5_drain_rx();
	SERCOM5->SPI.DATA.reg = data;
}

void spi5_transmit(uint8_t data)
{
    // Wait until the TX buffer is empty
    while (!SERCOM5->SPI.INTFLAG.bit.DRE);

    // Write the data out
    SERCOM5->SPI.DATA.reg = data;
	
	while (!SERCOM5->SPI.INTFLAG.bit.TXC) {
		if (SERCOM5->SPI.INTFLAG.bit.RXC) {
			(void)SERCOM5->SPI.DATA.reg;
		}
	}
	spi5_drain_rx();
}




uint8_t spi5_receive()
{
	// Call transfer with dummy data (receiving always needs the clock)
	return spi5_transfer(0xFF);
}

uint8_t spi5_transfer(uint8_t data)
{
	// 1. Wait until the transmit buffer is empty
	while (!SERCOM5->SPI.INTFLAG.bit.DRE);
	
	// 2. Write the data, which starts the clock and the transfer
	SERCOM5->SPI.DATA.reg = data;

	// 3. Wait until the byte has been completely received
	while (!SERCOM5->SPI.INTFLAG.bit.RXC);
	
	// 4. Read and return the data (RXC guarantees it is ready)
	return (uint8_t)SERCOM5->SPI.DATA.reg;
} 

void spi5_transmit_buffer(const uint8_t *buf, uint16_t len)
{
	for (uint16_t i = 0; i < len; i++) {
		// Wait for room in the TX buffer
		while (!SERCOM5->SPI.INTFLAG.bit.DRE);
		
		// Write the data out
		SERCOM5->SPI.DATA.reg = buf[i];
		if (SERCOM5->SPI.INTFLAG.bit.RXC) {
			(void)SERCOM5->SPI.DATA.reg;
		}
	}
	
	// When the WHOLE buffer is sent, wait until the last byte has shifted out
	while (!SERCOM5->SPI.INTFLAG.bit.TXC);
	spi5_drain_rx();
}





void spi5_receive_buffer(uint8_t *buf, uint16_t len)
{
	if (len == 0) return;

	uint16_t tx_count = 0;
	uint16_t rx_count = 0;

	// 1. Fill the SAMD21 transmit buffer (max. 2 bytes in flight)
	while (tx_count < len && tx_count < 2) {
		while (!SERCOM5->SPI.INTFLAG.bit.DRE);
		SERCOM5->SPI.DATA.reg = 0xFF; // dummy byte to start the clock
		tx_count++;
	}

	// 2. Run the read/write pipeline
	while (rx_count < len) {
		// Wait until the byte has been completely received
		while (!SERCOM5->SPI.INTFLAG.bit.RXC);
		
		// Store the received data right away
		buf[rx_count++] = (uint8_t)SERCOM5->SPI.DATA.reg;

		// If bytes are still to be sent, push a new dummy into the buffer
		if (tx_count < len) {
			while (!SERCOM5->SPI.INTFLAG.bit.DRE);
			SERCOM5->SPI.DATA.reg = 0xFF;
			tx_count++;
		}
	}
}

void spi5_transfer_buffer(const uint8_t *tx, uint8_t *rx, uint16_t len)
{
	for (uint16_t i = 0; i < len; i++) {
		rx[i] = spi5_transfer(tx[i]);
	}
}


// Overloaded transmit that pads to 256 bytes and discards RX on the fly
void spi5_transmit(const uint8_t *buf, uint16_t len, uint16_t total_pad_len)
{
	for (uint16_t i = 0; i < total_pad_len; i++) {
		// Wait for room in the TX buffer
		while (!SERCOM5->SPI.INTFLAG.bit.DRE);
		
		// Send the data while it lasts, then pad with 0xFF.
		if (i < len) {
			SERCOM5->SPI.DATA.reg = buf[i];
			} else {
			SERCOM5->SPI.DATA.reg = 0xFF; // padding generated on the fly
		}
		if (SERCOM5->SPI.INTFLAG.bit.RXC) {
			(void)SERCOM5->SPI.DATA.reg;
		}
	}
	
	// When all 256 bytes are pushed, wait until the last bit has shifted out
	while (!SERCOM5->SPI.INTFLAG.bit.TXC);
	spi5_drain_rx();
}


#endif
