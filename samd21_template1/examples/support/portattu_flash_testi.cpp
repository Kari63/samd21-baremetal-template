/*
 * portattu_flash_testi.cpp
 *
 * Created: 8.9.2026 18.39.50
 *  Author: Kari63
 */ 
#include "project_config.h"
#if USE_QSPI_FLASH



#include "systick.h"
#include "sst26_flash.h"
#include "examples/support/portattu_flash_testi.h"


// Local microsecond delay
static inline void local_asm_delay_us(uint32_t us) {
	volatile uint32_t count = us * 8;
	while (count) {
		count--;
	}
}


void portattu_flash_testi(void)
{




// 1. Short settling time before wake-up
local_asm_delay_us(1000);

// 2. Read the IDs with the ported function (by reference)
int manufacturer_id = 0, device_type = 0, device_id = 0;
flash_jedec_id_read(manufacturer_id, device_type, device_id);

// Store them in the global debug variables as before
sst26_mid2 = manufacturer_id;
sst26_d02  = device_type;
sst26_d12  = device_id;

// 3. Software reset
flash_reset_en();
local_asm_delay_us(5);
flash_reset();
local_asm_delay_us(100);

// 4. Wake up from deep power-down
flash_write_resume(); // DPD EXIT / resume command (0x30)
local_asm_delay_us(20);

// 5. WREN + GBULK (clears the 0x1E protection state)
flash_wren();
local_asm_delay_us(2);
flash_global_block_protection_unlock();
local_asm_delay_us(5);

// 6. WRR (0x01): force SR1 and SR2 to zero to clear the error state
flash_wren();
local_asm_delay_us(2);
flash_write_status_register(0x0000, 2); // writes 0x0000 (SR1=00, SR2=00)

// Wait until the register write is done
flash_wait_busy();

// 7. Unlock the blocks again after the WRR command
flash_wren();
local_asm_delay_us(2);
flash_global_block_protection_unlock();
local_asm_delay_us(5);

// --- WRITE AND ERASE TEST USING THE SAME SEQUENCE ---
uint32_t test_osoite = 0x000000;

// Forced unlock before the erase (so that nothing is rejected after an ELF run)
flash_wren();
local_asm_delay_us(2);
flash_global_block_protection_unlock();
local_asm_delay_us(5);

// Set the write enable latch for the erase
flash_wren();
local_asm_delay_us(2);

// 4 KB sector erase (0x20) with the ported function
flash_sector_erase(test_osoite);

// 25 ms delay: WIP does not rise immediately after erase (SST26 quirk)
delay_ms(25);

// Wait until the erase is complete
flash_wait_busy();

// Prepare the test bytes
uint8_t tx_puskuri[4] = { 0xAA, 0xBB, 0xCC, 0xDD };
uint8_t rx_puskuri[4] = { 0 };

// Set the write enable latch for page program
flash_wren();
local_asm_delay_us(2);

// Page Program (0x02). This ported function uses
// spi5_transmit_buffer, which discards the RX bytes on the fly.
flash_page_program(test_osoite, tx_puskuri,4);

// Wait until the write is done
flash_wait_busy();


// Read the 4 bytes back with the ported continuous read
flash_read_cont(test_osoite, 4, rx_puskuri);

// Copy the bytes to the array shown on the LCD and in PuTTY
for (uint8_t i = 0; i < 4; i++) {
	sst26_prog_rx[i] = rx_puskuri[i];
}

// Read the final register values for LCD_update()
sst26_sr1  = flash_read_status_register();
sst26_sr2  = flash_read_status_register(); // (could later read SR2 separately if needed)
sst26_rdcr = flash_read_configuration_register(); // this already reads (and discards) both bytes
}

#endif