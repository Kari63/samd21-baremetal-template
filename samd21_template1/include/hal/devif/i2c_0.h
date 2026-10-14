/*
 * i2c_0.h
 *
 * Created: 19.8.2026 19.04.14
 *  Author: Kari63
 */ 

#pragma once

#include "samd21.h"
#include <stdint.h>


void i2c_start_write(uint8_t addr);
void i2c_write(uint8_t data);
void i2c_stop(void);
void i2c_start_read(uint8_t addr);
uint8_t i2c_read_ack(void);
uint8_t i2c_read_nak(void);
uint8_t i2c_read_nak(uint8_t addr);