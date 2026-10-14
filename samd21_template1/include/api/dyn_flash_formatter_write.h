


#pragma once

#include <cstdint>
#include <cstdbool>

// Function declarations, C++ style (character first, backend last)
void flash_init(void);
void flash_erase(void);
//void flash_dyn_packer_init(uint32_t start_addr);
void flash_dyn_packer_init(uint32_t start_addr = 0xFFFFFFFF);

void flash_dyn_packer_putc(char c);
void flash_dyn_packer_flush(void);
void flash_tallenna_kirjoituspaikka(void);
uint32_t flash_hae_viimeisin_kirjoituspaikka(void);



void flash_dyn_reader(std::uint32_t start_addr, std::uint8_t backend);
void backend_print(const char *s);

void flash_raw_dump(uint32_t addr, uint32_t len);



