/*
 * sst26_flash.cpp - SST26VF/WF SPI flash, full command-level driver
 *
 * One function per SST26 instruction (read, erase, program, protection,
 * reset, SFDP ...). Used by the SPI flash dynamic packer
 * (dyn_flash_formatter_write.cpp) and the flash examples.
 *
 * The project also has a lightweight, self-contained alternative in
 * flash_wr.cpp, which writes the command sequences directly on SPI5
 * without this driver. Both are kept on purpose.
 *
 * SST26 quirks: see README.md ("SST26VF/WF SPI flash").
 */

#include "project_config.h"
#if USE_QSPI_FLASH

#include "sst26_flash.h"
#include "samd21.h"

uint8_t flash_read_status_register()
{
	uint8_t byte = 0;
	spi5_flash_cs_low();
	spi5_transfer(0x05);         /* send RDSR command */
	byte = spi5_transfer(0xFF);  /* receive byte */
	spi5_flash_cs_high();
	return byte;
}

uint8_t flash_read_configuration_register()
{
	uint8_t byte = 0;
	spi5_flash_cs_low();
	spi5_transfer(0x35);         /* send RDCR command */
	byte = spi5_transfer(0xFF);  /* receive byte */
	spi5_flash_cs_high();
	return byte;
}

void flash_wren()
{
	spi5_flash_cs_low();
	spi5_transfer(0x06);         /* send WREN command */
	spi5_flash_cs_high();
}

void flash_wrdi()
{
	spi5_flash_cs_low();
	spi5_transfer(0x04);         /* send WRDI command */
	spi5_flash_cs_high();
}

void flash_jedec_id_read(int &manufacturer_id, int &device_type, int &device_id)
{
	spi5_flash_cs_low();
	spi5_transfer(0x9F);         /* send JEDEC ID command (9Fh) */
	manufacturer_id = spi5_transfer(0xFF);
	device_type     = spi5_transfer(0xFF);
	device_id       = spi5_transfer(0xFF);
	spi5_flash_cs_high();
}

uint8_t flash_read(uint32_t dst)
{
	uint8_t byte = 0;
	spi5_flash_cs_low();
	spi5_transfer(0x03);                        /* read command */
	spi5_transfer((dst >> 16) & 0xFF);          /* send 3 address bytes */
	spi5_transfer((dst >> 8) & 0xFF);
	spi5_transfer(dst & 0xFF);
	byte = spi5_transfer(0xFF);
	spi5_flash_cs_high();
	return byte;
}

void flash_read_cont(uint32_t dst, uint32_t no_bytes, uint8_t *read_data)
{
	spi5_flash_cs_low();
	spi5_transfer(0x03);                        /* read command */
	spi5_transfer((dst >> 16) & 0xFF);          /* send 3 address bytes */
	spi5_transfer((dst >> 8) & 0xFF);
	spi5_transfer(dst & 0xFF);
	
	spi5_receive_buffer(read_data, no_bytes);
	
	spi5_flash_cs_high();
}

uint8_t flash_highspeed_read(uint32_t dst)
{
	uint8_t byte = 0;
	spi5_flash_cs_low();
	spi5_transfer(0x0B);                        /* highspeed read command */
	spi5_transfer((dst >> 16) & 0xFF);          /* send 3 address bytes */
	spi5_transfer((dst >> 8) & 0xFF);
	spi5_transfer(dst & 0xFF);
	spi5_transfer(0xFF);                        /* dummy byte */
	byte = spi5_transfer(0xFF);
	spi5_flash_cs_high();
	return byte;
}

void flash_highspeed_read_cont(uint32_t dst, uint32_t no_bytes, uint8_t *read_data)
{
	spi5_flash_cs_low();
	spi5_transfer(0x0B);                        /* highspeed read command */
	spi5_transfer((dst >> 16) & 0xFF);          /* send 3 address bytes */
	spi5_transfer((dst >> 8) & 0xFF);
	spi5_transfer(dst & 0xFF);
	spi5_transfer(0xFF);                        /* dummy byte */
	
	spi5_receive_buffer(read_data, no_bytes);
	
	spi5_flash_cs_high();
}

void flash_page_program(uint32_t dst, const uint8_t *prog_data, uint32_t len) // FIX: length parameter added
{
	spi5_flash_cs_low();
	spi5_transfer(0x02);                        /* send Page Program command */
	spi5_transfer((dst >> 16) & 0xFF);          /* send 3 address bytes */
	spi5_transfer((dst >> 8) & 0xFF);
	spi5_transfer(dst & 0xFF);
	
	spi5_transmit_buffer(prog_data, len);       // FIX: use caller-provided length
	
	spi5_flash_cs_high();
}

void flash_chip_erase()
{
	spi5_flash_cs_low();
	spi5_transfer(0xC7);                        /* send Chip Erase command */
	spi5_flash_cs_high();
}

void flash_sector_erase(uint32_t dst)
{
	spi5_flash_cs_low();
	spi5_transfer(0x20);                        /* send Sector Erase command */
	spi5_transfer((dst >> 16) & 0xFF);          /* send 3 address bytes */
	spi5_transfer((dst >> 8) & 0xFF);
	spi5_transfer(dst & 0xFF);
	spi5_flash_cs_high();
}

void flash_block_erase(uint32_t dst)
{
	spi5_flash_cs_low();
	spi5_transfer(0xD8);                        /* send Block Erase command */
	spi5_transfer((dst >> 16) & 0xFF);          /* send 3 address bytes */
	spi5_transfer((dst >> 8) & 0xFF);
	spi5_transfer(dst & 0xFF);
	spi5_flash_cs_high();
}

void flash_global_block_protection_unlock()
{
	flash_wren();                               // FIX: WREN required before GBULK
	spi5_flash_cs_low();
	spi5_transfer(0x98);                        /* Global Block Protection Unlock */
	spi5_flash_cs_high();
}

void flash_wait_busy()
{
	for (volatile int d = 0; d < 25000; d++) { __NOP(); } // FIX: allow WIP to rise after erase/program
	while ((flash_read_status_register() & 0x01) == 0x01)
	{
		for (volatile int d = 0; d < 100; d++) { __NOP(); }
	}
}


uint8_t flash_sfdp_read(uint32_t dst)
{
	uint8_t byte = 0;
	spi5_flash_cs_low();
	spi5_transfer(0x5A);                        /* read SFDP command */
	spi5_transfer((dst >> 16) & 0xFF);          /* send 3 address bytes */
	spi5_transfer((dst >> 8) & 0xFF);
	spi5_transfer(dst & 0xFF);
	spi5_transfer(0xFF);                        /* dummy byte */
	byte = spi5_transfer(0xFF);
	spi5_flash_cs_high();
	return byte;
}

void flash_noop() { spi5_flash_cs_low(); spi5_transfer(0x00); spi5_flash_cs_high(); }
void flash_reset_en() { spi5_flash_cs_low(); spi5_transfer(0x66); spi5_flash_cs_high(); }
void flash_reset() { spi5_flash_cs_low(); spi5_transfer(0x99); spi5_flash_cs_high(); }
void flash_write_suspend() { spi5_flash_cs_low(); spi5_transfer(0xB0); spi5_flash_cs_high(); }
void flash_write_resume() { spi5_flash_cs_low(); spi5_transfer(0x30); spi5_flash_cs_high(); }

void flash_write_status_register(uint16_t data1, uint8_t /*datalen*/)
{
	spi5_flash_cs_low();
	spi5_transfer(0x01);
	spi5_transfer((data1 >> 8) & 0xFF);  // SR1
	spi5_transfer(data1 & 0xFF);         // SR2
	spi5_flash_cs_high();
}

#endif

