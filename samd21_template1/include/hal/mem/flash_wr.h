/*
 * flash_wr.h
 *
 * SST26VF/WF SPI flash, lightweight self-contained driver
 * (full command-level driver: sst26_flash.h)
 *
 * Created: 7.9.2026 16.00.58
 *  Author: Kari63
 */ 


#pragma once

extern volatile uint8_t sst26_mid2;
extern volatile uint8_t sst26_d02;
extern volatile uint8_t sst26_d12;
extern volatile uint8_t sst26_sr1;
extern volatile uint8_t sst26_sr2;
extern volatile uint8_t sst26_rdcr;
extern volatile uint8_t sst26_prog_rx[4];
extern volatile uint8_t sst26_prog_tx[4];


void flash_sst26_smoke_test(void);
void flash_sst26_erase_program_test(void);
void flash_sst26_sfdp_test(void);


