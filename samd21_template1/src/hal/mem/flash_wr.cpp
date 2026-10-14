/*
 * flash_wr.cpp - SST26VF/WF SPI flash, lightweight self-contained driver
 *
 * Writes the complete command sequences (wake-up, reset, unlock, erase,
 * page program, read back, SFDP) directly on SPI5, with no dependency on
 * the full driver in sst26_flash.cpp. Useful as a minimal reference and
 * for bring-up / smoke tests. Both drivers are kept on purpose.
 *
 * SST26 quirks: see README.md ("SST26VF/WF SPI flash").
 */

#include "project_config.h"
#if USE_QSPI_FLASH



#include "samd21.h"
#include <stdint.h>
#include "spi5.h"
#include "flash_wr.h"
#include "systick.h"



static inline void asm_delay_us(uint32_t us) {
	// At 48 MHz one microsecond takes about 7-8 iterations of this volatile loop
	volatile uint32_t count = us * 8;
	while (count) {
		count--;
	}
}


void flash_sst26_smoke_test(void)
{
	asm_delay_us(1000);
	/* --- RDID --- */
	spi5_flash_cs_low();
	spi5_transmit(0x9F);
	sst26_mid2 = spi5_receive();
	sst26_d02  = spi5_receive();
	sst26_d12  = spi5_receive();
	spi5_flash_cs_high();

	/* --- Software Reset --- */
	spi5_flash_cs_low();
	spi5_transmit(0x66);   // RSTEN
	spi5_flash_cs_high();
	asm_delay_us(5); 

	spi5_flash_cs_low();
	spi5_transmit(0x99);   // RST
	spi5_flash_cs_high();
	asm_delay_us(100);
	

	/* --- Exit Deep Power-Down --- */
	spi5_flash_cs_low();
	spi5_transmit(0xAB);   // DPD EXIT
	spi5_flash_cs_high();
	asm_delay_us(20);   

	/* --- Global Block Unlock --- */
	spi5_flash_cs_low();
	spi5_transmit(0x06);   // WREN
	spi5_flash_cs_high();
	asm_delay_us(2);

	spi5_flash_cs_low();
	spi5_transmit(0x98);   // GBULK
	spi5_flash_cs_high();
	asm_delay_us(5);



	/// --- WRR (SR1 + SR2) --- 
	spi5_flash_cs_low();
	spi5_transmit(0x06);   // WREN
	spi5_flash_cs_high();

	spi5_flash_cs_low();
	spi5_transmit(0x01);   // WRR
	spi5_transmit(0x00);   // SR1
	spi5_transmit(0x00);   // SR2
	spi5_flash_cs_high();  

	// --- Poll WIP --- 
	do {
		spi5_flash_cs_low();
		spi5_transmit(0x05);
		sst26_sr1 = spi5_receive();
		spi5_flash_cs_high();
	} while (sst26_sr1 & 0x01);

	// --- Global Block Unlock --- 
	spi5_flash_cs_low();
	spi5_transmit(0x06);   // WREN
	spi5_flash_cs_high();
	asm_delay_us(2);

	spi5_flash_cs_low();
	spi5_transmit(0x98);   // GBULK
	spi5_flash_cs_high();
	asm_delay_us(5);


	/* --- Final registers --- */
	spi5_flash_cs_low();
	spi5_transmit(0x05);
	sst26_sr1 = spi5_receive();
	spi5_flash_cs_high();

	spi5_flash_cs_low();
	spi5_transmit(0x3F);
	sst26_sr2 = spi5_receive();
	spi5_flash_cs_high();


	
	spi5_flash_cs_low();
	spi5_transmit(0x35);
	sst26_rdcr = spi5_receive(); // first byte
	(void)spi5_receive();        // second byte (must be read out)
	spi5_flash_cs_high();
	
	// added
	/* --- 1. WRITE ENABLE FOR THE ERASE --- */
	
	// --- FORCED UNLOCK FIRST, SO THE CHIP DOES NOT REJECT THE ERASE AFTER AN ELF RUN ---
	spi5_flash_cs_low();
	spi5_transmit(0x06);   // WREN
	spi5_flash_cs_high();
	
	asm_delay_us(5);
	spi5_flash_cs_low();
	spi5_transmit(0x98);   // Global Block Unlock (removes the protection)
	spi5_flash_cs_high();
	asm_delay_us(5);

	/* --- 1. WRITE ENABLE FOR THE ERASE --- */
	spi5_flash_cs_low();
	spi5_transmit(0x06);
	spi5_flash_cs_high();
	asm_delay_us(2);

	/* --- 2. CHIP ERASE (0xC7) --- */
	spi5_flash_cs_low();
	spi5_transmit(0xC7);
	spi5_flash_cs_high();
	
	delay_ms(25);
	delay_ms(25);
	
	/* --- 3. WIP‑poll  */
	do {
		spi5_flash_cs_low();
		spi5_transmit(0x05);
		sst26_sr1 = spi5_receive();
		spi5_flash_cs_high();
		for (volatile int d = 0; d < 100; d++) { __NOP(); }
	} while (sst26_sr1 & 0x01);
		
}

void flash_sst26_erase_program_test(void)
{	
	uint32_t addr = 0x000000;


	// --- FORCED UNLOCK FIRST, SO THE CHIP DOES NOT REJECT THE ERASE AFTER AN ELF RUN ---
	spi5_flash_cs_low();
	spi5_transmit(0x06);   // WREN
	spi5_flash_cs_high();
	
	asm_delay_us(5);
	spi5_flash_cs_low();
	spi5_transmit(0x98);   // Global Block Unlock (removes the protection)
	spi5_flash_cs_high();
	asm_delay_us(5);

	/* --- 1. WRITE ENABLE FOR THE ERASE --- */
	spi5_flash_cs_low();
	spi5_transmit(0x06);
	spi5_flash_cs_high();
	asm_delay_us(2);

	/* --- 2. CHIP ERASE (0xC7) --- */
	spi5_flash_cs_low();
	spi5_transmit(0xC7);
	spi5_flash_cs_high();
	
	delay_ms(25);
	/* --- 3. SECTOR ERASE 4 KB (0x20) --- */
/*	spi5_flash_cs_low();
	spi5_transmit(0x20);   // Sector Erase command (smallest erase unit)
	spi5_transmit((addr >> 16) & 0xFF); // address MSB first
	spi5_transmit((addr >> 8)  & 0xFF);
	spi5_transmit((addr >> 0)  & 0xFF); // address LSB last
	spi5_flash_cs_high();
 */	delay_ms(25);
	
	/* --- 3. WIP‑poll  */
	do {
		spi5_flash_cs_low();
		spi5_transmit(0x05);
		sst26_sr1 = spi5_receive();
		spi5_flash_cs_high();
		for (volatile int d = 0; d < 100; d++) { __NOP(); }
	} while (sst26_sr1 & 0x01);
	
	addr = 0x000000;
	// 4-byte buffer
	uint8_t txbuf[4] = { 0xAB, 0xBA, 0xC2, 0xC3 };
	uint8_t rxbuf[4] = { 0 };

	/* --- 4. WRITE ENABLE FOR THE WRITE --- */
	spi5_flash_cs_low();
	spi5_transmit(0x06);
	spi5_flash_cs_high();

	/* --- 5. PAGE PROGRAM (0x02) --- */
	spi5_flash_cs_low();
	spi5_transmit(0x02);
	spi5_transmit((addr >> 16) & 0xFF);
	spi5_transmit((addr >> 8)  & 0xFF);
	spi5_transmit((addr >> 0)  & 0xFF);

	// Send only the 4 bytes with a plain loop
	for (uint8_t i = 0; i < 4; i++) {
		spi5_transmit(txbuf[i]);
	}
	spi5_flash_cs_high();

	/* --- 6. WIP‑poll --- */
	do {
		spi5_flash_cs_low();
		spi5_transmit(0x05);
		sst26_sr1 = spi5_receive();
		spi5_flash_cs_high();
	} while (sst26_sr1 & 0x01);

	/* --- 7. READ (0x03) --- */
	spi5_flash_cs_low();
	spi5_transmit(0x03);
	spi5_transmit((addr >> 16) & 0xFF);
	spi5_transmit((addr >> 8)  & 0xFF);
	spi5_transmit((addr >> 0)  & 0xFF);

	spi5_receive_buffer(rxbuf, 4);
	spi5_flash_cs_high();

	for (uint8_t i = 0; i < 4; i++) {
		sst26_prog_rx[i] = rxbuf[i];
	}
}



void flash_sst26_sfdp_test(void)
{
	uint8_t rxbuf[4] = {0};

	/* --- READ SFDP (0x5A) --- */
	spi5_flash_cs_low();
	
	spi5_transmit(0x5A);                        // SFDP command
	spi5_transmit(0x00);         // address A23-A16
	spi5_transmit(0x00);         // address A15-A8
	spi5_transmit(0x00);			 // address A7-A0
	(void)spi5_transfer(0xFF); 


	rxbuf[0] = spi5_receive(); // should be 'S' (0x53)
	rxbuf[1] = spi5_receive(); // 'F' (0x46)
	rxbuf[2] = spi5_receive(); // 'D' (0x44)
	rxbuf[3] = spi5_receive(); // 'P' (0x50)

	spi5_flash_cs_high();


	extern volatile uint8_t sst26_prog_rx[4];
	for (uint8_t i = 0; i < 4; i++) {
		sst26_prog_rx[i] = rxbuf[i];
	}
}


#endif