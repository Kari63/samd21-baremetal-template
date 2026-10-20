/*
 * pcf8574.h - PCF8574 8-bit I2C I/O expander
 *
 * Copyright 2026 Kari63
 * SPDX-License-Identifier: Apache-2.0
 *
 * Quasi-bidirectional port: output '1' = weak pull-up, so the pin can be
 * used as an input. Only pins written to '1' can therefore be read.
 * The output state cannot be read back from the chip, so a copy is kept
 * in memory.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>


/* Address = PCF8574_ADDRBASE + deviceid, deviceid = A2..A0 (0..7).
   PCF8574: 0x20, PCF8574A: 0x38. */
#define PCF8574_ADDRBASE    0x20
#define PCF8574_MAXDEVICES  8


void    pcf8574_init(void);
void    pcf8574_setoutput(uint8_t deviceid, uint8_t data);
void    pcf8574_setoutputs(uint8_t deviceid, const uint8_t *data, uint8_t count);
bool    pcf8574_write_frame(uint8_t deviceid, const uint8_t *data, uint16_t count);  /* USE_I2C_DMA: non-blocking */
uint8_t pcf8574_getoutput(uint8_t deviceid);
uint8_t pcf8574_getinput(uint8_t deviceid);
