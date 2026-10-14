/*
 * sst26_flash.h - SST26VF/WF SPI flash, full command-level driver
 * (lightweight alternative: flash_wr.h)
 */
#ifndef SST26_FLASH_H
#define SST26_FLASH_H

#include "samd21.h"
#include "spi5.h"



// Basic operations and registers
uint8_t flash_read_status_register();
uint8_t flash_read_configuration_register();
void flash_wren();
void flash_wrdi();
void flash_jedec_id_read(int &manufacturer_id, int &device_type, int &device_id);

// Read operations
uint8_t flash_read(uint32_t dst);
void flash_read_cont(uint32_t dst, uint32_t no_bytes, uint8_t *read_data);
uint8_t flash_highspeed_read(uint32_t dst);
void flash_highspeed_read_cont(uint32_t dst, uint32_t no_bytes, uint8_t *read_data);

// Write and erase
void flash_page_program(uint32_t dst, const uint8_t *prog_data, uint32_t len);
void flash_chip_erase();
void flash_sector_erase(uint32_t dst);
void flash_block_erase(uint32_t dst);

// System and protection commands
void flash_noop();
void flash_reset_en();
void flash_reset();
void flash_write_suspend();
void flash_write_resume();
void flash_write_status_register(uint16_t data1, uint8_t datalen);
void flash_global_block_protection_unlock();
void flash_wait_busy();
uint8_t flash_sfdp_read(uint32_t dst);

#endif /* SST26_FLASH_H */
