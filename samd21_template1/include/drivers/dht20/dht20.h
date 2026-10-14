/*
 * dht20.h
 *
 * Created: 20.8.2026 19.02.15
 *  Author: Kari63
 */ 


#pragma once
#include <stdint.h>

void DHT20_Init(void);
uint8_t DHT20_Read(float *temperature, float *humidity);
