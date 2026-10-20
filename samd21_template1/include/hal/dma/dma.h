/*
 * dma.h
 *
 * Created: 18.9.2026 13.19.05
 *  Author: Kari63
 */ 
#include "samd21.h"
#include <stddef.h>
#include <stdbool.h>


#pragma once

//#define BLOCK_SIZE  64  // 64-byte page/block - divisible by 4
#define DMA_CHANNELS 12  // ch0 = EVSYS, ch1 = ADC, ch4-5 = SPI TX / RX-drain
#define I2C_DMA_CHANNEL 6  // SERCOM0 I2C TX (LCD). Above any channel with linked
                           // descriptors (ADC ping-pong): errata 1.7.2
#define ADC_DMA_CHANNEL 1
#define SPI_DMA_CHANNEL 4
#define ADC_DMA_BUFFER_SAMPLES 16

// Debug flags
extern volatile uint8_t dma_transfer_complete ;
extern volatile uint8_t dma_transfer_error ;
extern volatile uint32_t dma_tcmpl_count; // event counter: channel 0 (TC4 trigger) TCMPL hits
extern volatile uint32_t adc_dma_block_count;
extern volatile uint32_t adc_dma_error_count;


typedef struct {
	uint16_t btctrl;
	uint16_t btcnt;
	uint32_t srcaddr;
	uint32_t dstaddr;
	uint32_t descaddr;
} dmac_descriptor_t;

typedef void (*dmac_channel_cb_t)(uint8_t channel, uint8_t flags);

// void fill_test_pattern(void);

void dmac_init(void);
bool dmac_set_channel_callback(uint8_t channel, dmac_channel_cb_t cb);


// --- Channel 1: ADC RESULT -> two alternating sample buffers ---
void dmac_build_channel_adc_ping_pong(void);
void dmac_arm_channel_adc_trigger(void);

// --- Channel 4: SPI TX (SERCOM5) ---
// dmac_build_channel4_descriptor: fixed SPI_CHUNK_SIZE hardware test from the spi_tx_chunk buffer.
// dmac_build_channel4_color_block: general-purpose transfer of any data/len pair
//   (e.g. the display color fill calls it with its own buffer - dma.cpp knows
//   nothing about its caller).

void dmac_build_channel_color_block(const uint8_t *data, uint16_t len);
void dmac_arm_channel_spi_trigger(void);



// Starts one logical SPI transfer atomically:
void dmac_start_spi_transfer(const uint8_t *data, uint16_t len);

// --- General-purpose completion notification ---
// Called from DMAC_Handler (ISR context) when channel 4 has
// completed. ok=true on a normal TCMPL, ok=false if the
// channel got a TERR error during the transfer. dma.cpp itself does nothing to CS
// or the bus - that is the job of the registered callback (e.g. dma_tft.cpp).
typedef void (*dma_spi_complete_cb_t)(bool ok);
void dma_spi_set_complete_callback(dma_spi_complete_cb_t cb);


// --- I2C_DMA_CHANNEL: SERCOM0 I2C master TX (USE_I2C_DMA, LCD) ---
// Starts one TX block to SERCOM0 DATA. The callback (DMAC_Handler, ISR
// context) reports TCMPL (ok = true: the last byte is in DATA, the bus is
// still busy with it) or TERR (ok = false).
typedef void (*dma_i2c_complete_cb_t)(bool ok);
void dma_i2c_set_complete_callback(dma_i2c_complete_cb_t cb);
void dmac_start_i2c_transfer(const uint8_t *data, uint16_t len);
void dmac_stop_i2c_transfer(void);

void DMAC_Handler(void);
