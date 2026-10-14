/*
 * portattu_flash_testi.h
 *
 * Created: 8.9.2026 18.39.09
 *  Author: Kari63
 */ 
#pragma once

#include "sst26_flash.h"

 extern volatile uint8_t sst26_mid2;
 extern volatile uint8_t sst26_d02;
 extern volatile uint8_t sst26_d12;
 extern volatile uint8_t sst26_sr1;
 extern volatile uint8_t sst26_sr2;
 extern volatile uint8_t sst26_rdcr;
 extern volatile uint8_t sst26_prog_rx[4];
 extern volatile uint8_t sst26_prog_tx[4];

void portattu_flash_testi(void);