/*
 * i2c_0.h
 *
 * Created: 19.8.2026 19.04.14
 *  Author: Kari63
 */ 

#pragma once

#include "samd21.h"
#include "project_config.h"
#include <stdint.h>


void i2c_start_write(uint8_t addr);
void i2c_write(uint8_t data);
void i2c_stop(void);
void i2c_start_read(uint8_t addr);
uint8_t i2c_read_ack(void);
uint8_t i2c_read_nak(void);
uint8_t i2c_read_nak(uint8_t addr);

#define I2C_TIMEOUT_MS      5u      /* DMA write: + 125 us per byte */

/* Transfer API (same as the STM32 template's fmpi2c1): true = every byte
   ACKed and STOP sent, false = NACK, bus error, arbitration lost or
   timeout. Every wait has a timeout. Not from interrupts.
   i2c_write with n = 0: address probe (I2CSCAN). */
bool i2c_write(uint8_t addr, const uint8_t *data, uint8_t n);
bool i2c_read(uint8_t addr, uint8_t *data, uint8_t n);             /* n >= 1 */
bool i2c_write_read(uint8_t addr, const uint8_t *wdata, uint8_t wn,
                    uint8_t *rdata, uint8_t rn);                    /* repeated START */

/* USE_I2C_DMA: non-blocking write, n <= 255 (ADDR.LEN). Returns false if
   n = 0 or a write is still running. data must stay unchanged until
   i2c_dma_busy() is false. The polled functions above first wait for it. */
#if USE_I2C_DMA
bool i2c_write_dma(uint8_t addr, const uint8_t *data, uint8_t n);
bool i2c_dma_busy(void);            /* also ends the transfer on error / timeout */
void i2c_dma_wait(void);
bool i2c_dma_take_error(void);      /* NACK / bus error / timeout since the last call */
#else
static inline void i2c_dma_wait(void) {}
#endif
