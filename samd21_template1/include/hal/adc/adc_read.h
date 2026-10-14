/*
 * adc_read.h
 *
 * Created: 5.9.2026 0.01.27
 *  Author: Kari63
 */ 


#pragma once

#include <stdint.h>

extern volatile float adc_last_val;
extern volatile uint16_t raw;
extern volatile uint32_t adc_filtered_raw;


void adc_dma_samples_complete(const volatile uint16_t *samples, uint16_t count);
void adc_softirq_handler(void);
