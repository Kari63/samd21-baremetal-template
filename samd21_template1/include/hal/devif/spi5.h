/*
 * spi5.h
 *
 * Created: 21.8.2026 15.03.02
 *  Author: Kari63
 */ 

#pragma once
#include "samd21.h"
#include <stdint.h>

void spi5_flash_cs_low();
void spi5_flash_cs_high();

void spi5_transmit(uint8_t data);
void spi5_transmit_stream(uint8_t data);
void spi5_drain_rx(void);
uint8_t spi5_receive();                     // new
uint8_t spi5_transfer(uint8_t data);        // TX+RX combined
void spi5_transmit(const uint8_t *buf, uint16_t len, uint16_t total_pad_len);





void spi5_transmit_buffer(const uint8_t *buf, uint16_t len);
void spi5_receive_buffer(uint8_t *buf, uint16_t len);   // new
void spi5_transfer_buffer(const uint8_t *tx, uint8_t *rx, uint16_t len); // new


void spi5_flash_read_id(void);
void spi5_flash_unprotect(void);
void spi5_flash_erase_4kb(uint32_t address);
void spi5_flash_write_page(uint32_t address, const uint8_t *buf, uint16_t len);
void spi5_flash_read(uint32_t address, uint8_t *rx_buf, uint16_t len);