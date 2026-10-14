/*
 * dyn_flash_formatter_write.cpp - SPI flash dynamic packer (print() backend 5)
 *
 * Streams formatter output into 256-byte pages of the SST26 flash and keeps
 * the write position in an encoder sector, so writing continues after a
 * power loss without overwriting earlier data.
 *
 * Uses the full driver (sst26_flash.h) for erase/unlock/wait. Some
 * WREN + GBULK sequences are written here directly on SPI5 with explicit
 * delays, matching the sequence verified in the flash smoke test.
 *
 * working version 1, 13.9.26
 */

#include "project_config.h"

#if USE_QSPI_FLASH

#include "formatter.h"
#include "backend.h"
#include "dyn_flash_formatter_write.h"
#include "sst26_flash.h"  // <--- link to the SST26 drivers
#include "samd21.h"

 volatile uint8_t sst26_mid2;	
 volatile uint8_t sst26_d02;
 volatile uint8_t sst26_d12;
 volatile uint8_t sst26_sr1;
 volatile uint8_t sst26_sr2;
 volatile uint8_t sst26_rdcr;


// Global buffer variables
static uint8_t sst26_pack_buf[256] = { 0 };
static uint16_t sst26_pack_idx = 0;
static uint32_t sst26_dyn_addr = 0x000000;


// Coarse delay functions for SAMD21 (assumes ~48 MHz CPU clock)
static void packer_dyn_delay_us(uint32_t us)
{
	// One microsecond is about 12-16 NOP cycles at 48 MHz
	for (volatile uint32_t i = 0; i < us * 12; i++) {
		__NOP();
	}
}

static void delay_ms(uint32_t ms)
{
	for (volatile uint32_t i = 0; i < ms; i++) {
		packer_dyn_delay_us(1000);
	}
}

static inline void asm_delay_us(uint32_t us) {
	volatile uint32_t count = us * 8;
	while (count) {
		count--;
	}
}


void flash_dyn_packer_putc(char c)
{
	
	sst26_pack_buf[sst26_pack_idx++] = static_cast<uint8_t>(c);

	// As soon as the 256-byte page is full, program it to the chip
	if (sst26_pack_idx == 256)
	{
		flash_wren();
		packer_dyn_delay_us(2);
		flash_page_program(sst26_dyn_addr,
		(const uint8_t*)sst26_pack_buf,
		256);                 // BUG FIX: explicit length
		flash_wait_busy();

		// Move the address to the start of the next page
		sst26_dyn_addr += 256;

		// Check whether a new 4 KB sector boundary was reached
		if ((sst26_dyn_addr % 4096) == 0)
		{
			spi5_flash_cs_low(); spi5_transmit(0x06); spi5_flash_cs_high(); asm_delay_us(2); // WREN
			spi5_flash_cs_low(); spi5_transmit(0x98); spi5_flash_cs_high(); asm_delay_us(5); // GBULK
			
			spi5_flash_cs_low(); spi5_transmit(0x06); spi5_flash_cs_high(); asm_delay_us(2); // WREN
			flash_sector_erase(sst26_dyn_addr); delay_ms(25); flash_wait_busy();            // Erase

			// REQUIRED: unlock the freshly erased area again for dynamic writing
			spi5_flash_cs_low(); spi5_transmit(0x06); spi5_flash_cs_high(); asm_delay_us(2); // WREN
			spi5_flash_cs_low(); spi5_transmit(0x98); spi5_flash_cs_high(); asm_delay_us(5); // GBULK);
		}

		// Flush the buffer
		for (uint16_t i = 0; i < 256; i++) { sst26_pack_buf[i] = 0x00; }
		sst26_pack_idx = 0;
	}
}



void flash_dyn_packer_flush(void)
{
	if (sst26_pack_idx > 0)
	{
		flash_wren();
		packer_dyn_delay_us(2);
	//	uint32_t f_addr = sst26_dyn_addr - sst26_pack_idx;

		// Write only the used bytes, not all 256 (BUG FIX)
		flash_page_program(sst26_dyn_addr,
		(const uint8_t*)sst26_pack_buf,
		sst26_pack_idx);
		flash_wait_busy();
		
		sst26_dyn_addr += sst26_pack_idx;
	
		for (uint16_t i = 0; i < 256; i++) { sst26_pack_buf[i] = 0x00; }
		sst26_pack_idx = 0;
	}
	
	 flash_tallenna_kirjoituspaikka();
}


#define FLASH_ENCODER_SECTOR_ADDR  0xFFF000

void flash_tallenna_kirjoituspaikka(void)
{
	uint32_t talletettava_paikka = sst26_dyn_addr;
	uint8_t buf[4];

	// Split the 32-bit address into big-endian byte order
	buf[0] = (talletettava_paikka >> 24) & 0xFF;
	buf[1] = (talletettava_paikka >> 16) & 0xFF;
	buf[2] = (talletettava_paikka >> 8)  & 0xFF;
	buf[3] = talletettava_paikka         & 0xFF;

	// === RELIABLE UNLOCK SEQUENCE (per SST26 quirks 1 & 2) ===
	flash_wren();
	for (volatile int d = 0; d < 10; d++);
	flash_global_block_protection_unlock(); // WREN + GBULK
	for (volatile int d = 0; d < 20; d++);

	flash_wren();
	for (volatile int d = 0; d < 10; d++);
	flash_write_status_register(0x0000, 2); // clear the protections (SR1=00, SR2=00)
	flash_wait_busy();

	flash_wren();
	for (volatile int d = 0; d < 10; d++);
	flash_global_block_protection_unlock(); // final unlock
	for (volatile int d = 0; d < 20; d++);

	// 1. Erase the encoder sector
	flash_wren();
	flash_sector_erase(FLASH_ENCODER_SECTOR_ADDR);
	delay_ms(25); // WIP does not rise immediately after erase (SST26 quirk)
	flash_wait_busy();

	// 2. Write the 4-byte buffer to flash
	flash_wren();
	flash_page_program(FLASH_ENCODER_SECTOR_ADDR, buf, 4);
	flash_wait_busy();
}

uint32_t flash_hae_viimeisin_kirjoituspaikka(void)
{
	uint8_t buf[4] = { 0 };

	// Read the raw 4 bytes into the array
	flash_read_cont(FLASH_ENCODER_SECTOR_ADDR, 4, buf);

	// If the flash is blank (0xFFFFFFFF) or zero, start from the beginning
	if ((buf[0] == 0xFF && buf[1] == 0xFF && buf[2] == 0xFF && buf[3] == 0xFF) ||
	(buf[0] == 0x00 && buf[1] == 0x00 && buf[2] == 0x00 && buf[3] == 0x00))
	{
		return 0x000000;
	}

	// Assemble the 32-bit value byte by byte in the right order
	uint32_t luettu_paikka = ((uint32_t)buf[0] << 24) |
	((uint32_t)buf[1] << 16) |
	((uint32_t)buf[2] << 8)  |
	buf[3];
	return luettu_paikka;
}


void flash_dyn_reader(uint32_t start_addr, uint8_t backend)
{
	uint32_t addr = start_addr;
	uint8_t page[256];

	while (1)
	{
		// Read 256 bytes from flash
		flash_read_cont(addr, 256, page);

		// If the whole page is blank -> stop
		bool empty = true;
		for (int i = 0; i < 256; i++) {
			if (page[i] != 0x00 && page[i] != 0xFF) {
				empty = false;
				break;
			}
		}
		if (empty) {
			break;   // end of text
		}

		// Print the characters through the backend
		for (int i = 0; i < 256; i++) {
			char c = (char)page[i];

			if (c == 0x00 || c == (char)0xFF) {
				return;   // end of text
			}

			//	backend_putc(c, 4);
			fb_putc(4,  c);
			
			
		}

		addr += 256;  // next page
	}
}


void flash_raw_dump(uint32_t addr, uint32_t len)
{
	uint8_t buf[256];

	while (len)
	{
		uint32_t chunk = (len > 256) ? 256 : len;

		flash_read_cont(addr, chunk, buf);

		print("\r\n%06lx : ", addr, 4);

		for(uint32_t i = 0; i < chunk; i++)
		{
			print("%02X ", buf[i], 4);
		}

		addr += chunk;
		len  -= chunk;
	}

	print("\r\n", 4);
}


void backend_print(const char *s)
{
	while (*s) {
		flash_dyn_packer_putc(*s++);
	}
}


void flash_dyn_packer_init(uint32_t start_addr)
{
	// If no parameter was given (or it is the auto flag), look up the position
	if (start_addr == 0xFFFFFFFF)
	{
		start_addr = flash_hae_viimeisin_kirjoituspaikka();
	}
	
	sst26_dyn_addr = start_addr;
	sst26_pack_idx = 0;

	// Clear the RAM buffer
	for (uint16_t i = 0; i < 256; i++) {
		sst26_pack_buf[i] = 0x00;
	}

	// ========================================================================
	// HARDWARE INIT: same sequence as the working smoke test
	// ========================================================================
	
	// Required start-up delay for the supplies to settle
	packer_dyn_delay_us(1000);

	// 1. RDID wake-up (synchronizes the bus, required for SST26)
	spi5_flash_cs_low();
	spi5_transmit(0x9F);
	(void)spi5_receive(); // discard MID
	(void)spi5_receive(); // discard DID1
	(void)spi5_receive(); // discard DID2
	spi5_flash_cs_high();

	// 2. Software Reset
	spi5_flash_cs_low(); spi5_transmit(0x66); spi5_flash_cs_high(); // RSTEN
	packer_dyn_delay_us(5);
	spi5_flash_cs_low(); spi5_transmit(0x99); spi5_flash_cs_high(); // RST
	packer_dyn_delay_us(100);

	// 3. Exit Deep Power-Down
	spi5_flash_cs_low(); spi5_transmit(0xAB); spi5_flash_cs_high(); // DPD EXIT
	packer_dyn_delay_us(20);

	// 4. Global Block Unlock (GBULK) before the status register
	spi5_flash_cs_low(); spi5_transmit(0x06); spi5_flash_cs_high(); // WREN
	packer_dyn_delay_us(2);
	spi5_flash_cs_low(); spi5_transmit(0x98); spi5_flash_cs_high(); // GBULK
	packer_dyn_delay_us(5);

	// 5. WRR (clear status registers SR1 + SR2 as one exact 2-byte sequence)
	spi5_flash_cs_low(); spi5_transmit(0x06); spi5_flash_cs_high(); // WREN
	spi5_flash_cs_low();
	spi5_transmit(0x01);   // WRR command
	spi5_transmit(0x00);   // SR1 = 0 (this clears the WPEN protection)
	spi5_transmit(0x00);   // SR2 = 0
	spi5_flash_cs_high();

	// Wait until the status register write is done (WIP polling)
	uint8_t status_sr1 = 0;
	do {
		spi5_flash_cs_low(); spi5_transmit(0x05);
		status_sr1 = spi5_receive(); spi5_flash_cs_high();
	} while (status_sr1 & 0x01);

	// 6. Final Global Block Unlock (WRR always locks the blocks again)
	spi5_flash_cs_low(); spi5_transmit(0x06); spi5_flash_cs_high(); // WREN
	packer_dyn_delay_us(2);
	spi5_flash_cs_low(); spi5_transmit(0x98); spi5_flash_cs_high(); // GBULK
	packer_dyn_delay_us(5);

	// ========================================================================
	// SECTOR ERASE LOGIC
	// ========================================================================
	// Erase the sector ONLY when starting at its beginning (not when resuming in the middle after a power loss)
	if ((sst26_dyn_addr % 4096) == 0)
	{
		spi5_flash_cs_low(); spi5_transmit(0x06); spi5_flash_cs_high(); // WREN
		packer_dyn_delay_us(2);
		
		// Low-level sector erase
		flash_sector_erase(sst26_dyn_addr);
		
		delay_ms(25); // WIP does not rise immediately after erase (SST26 quirk)
		flash_wait_busy();
		
		// Make sure once more that the blocks stay unlocked after the erase
		spi5_flash_cs_low(); spi5_transmit(0x06); spi5_flash_cs_high(); // WREN
		packer_dyn_delay_us(2);
		spi5_flash_cs_low(); spi5_transmit(0x98); spi5_flash_cs_high(); // GBULK
		packer_dyn_delay_us(5);
	}
}


void flash_erase(void)
{
	asm_delay_us(1000);

	/* --- RDID --- */
	spi5_flash_cs_low(); spi5_transmit(0x9F); sst26_mid2 = spi5_receive(); sst26_d02 = spi5_receive(); sst26_d12 = spi5_receive(); spi5_flash_cs_high();

	/* --- Software Reset --- */
	spi5_flash_cs_low(); spi5_transmit(0x66); spi5_flash_cs_high(); asm_delay_us(5);
	spi5_flash_cs_low(); spi5_transmit(0x99); spi5_flash_cs_high(); asm_delay_us(100);

	/* --- Exit Deep Power-Down --- */
	spi5_flash_cs_low(); spi5_transmit(0xAB); spi5_flash_cs_high(); asm_delay_us(20);

	/* --- Global Block Unlock --- */
	spi5_flash_cs_low(); spi5_transmit(0x06); spi5_flash_cs_high(); asm_delay_us(2);
	spi5_flash_cs_low(); spi5_transmit(0x98); spi5_flash_cs_high(); asm_delay_us(5);

	/* --- WRR (SR1 + SR2) --- */
	spi5_flash_cs_low(); spi5_transmit(0x06); spi5_flash_cs_high();
	spi5_flash_cs_low(); spi5_transmit(0x01); spi5_transmit(0x00); spi5_transmit(0x00); spi5_flash_cs_high();

	/* --- Poll WIP --- */
	do { spi5_flash_cs_low(); spi5_transmit(0x05); sst26_sr1 = spi5_receive(); spi5_flash_cs_high(); } while (sst26_sr1 & 0x01);

	/* --- Global Block Unlock --- */
	spi5_flash_cs_low(); spi5_transmit(0x06); spi5_flash_cs_high(); asm_delay_us(2);
	spi5_flash_cs_low(); spi5_transmit(0x98); spi5_flash_cs_high(); asm_delay_us(5);

	/* --- Final registers --- */
	spi5_flash_cs_low(); spi5_transmit(0x05); sst26_sr1 = spi5_receive(); spi5_flash_cs_high();
	spi5_flash_cs_low(); spi5_transmit(0x3F); sst26_sr2 = spi5_receive(); spi5_flash_cs_high();

	spi5_flash_cs_low(); spi5_transmit(0x35); sst26_rdcr = spi5_receive(); (void)spi5_receive(); spi5_flash_cs_high();

	/* --- FORCED UNLOCK FIRST, SO THE CHIP DOES NOT REJECT THE ERASE AFTER AN ELF RUN --- */
	spi5_flash_cs_low(); spi5_transmit(0x06); spi5_flash_cs_high(); asm_delay_us(5);
	spi5_flash_cs_low(); spi5_transmit(0x98); spi5_flash_cs_high(); asm_delay_us(5);

	/* --- WRITE ENABLE FOR THE ERASE --- */
	spi5_flash_cs_low(); spi5_transmit(0x06); spi5_flash_cs_high(); asm_delay_us(2);

	/* --- CHIP ERASE (0xC7) --- */
	spi5_flash_cs_low(); spi5_transmit(0xC7); spi5_flash_cs_high();
	delay_ms(25); delay_ms(25);

	/* --- WIP‑poll --- */
	do {
		spi5_flash_cs_low(); spi5_transmit(0x05); sst26_sr1 = spi5_receive(); spi5_flash_cs_high();
		for (volatile int d = 0; d < 100; d++) { __NOP(); }
	} while (sst26_sr1 & 0x01);
}

#endif

