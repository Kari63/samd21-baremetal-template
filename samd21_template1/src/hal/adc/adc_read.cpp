/*
 * adc_read.cpp
 *
 * Created: 5.9.2026 0.01.54
 *  Author: Kari63
 */ 
#include "project_config.h"

#if USE_ADC

#include "samd21.h"
#include <stdint.h>
#include "adc_read.h"
#include "softirq.h"


volatile uint16_t raw;
volatile uint32_t adc_filtered_raw = 0; // filtered
volatile float adc_last_val;

extern "C" void ADC_Handler(void)
{
	if (ADC->INTFLAG.bit.RESRDY) {
		ADC->INTFLAG.reg = ADC_INTFLAG_RESRDY;
	}
}

void adc_dma_samples_complete(const volatile uint16_t *samples, uint16_t count)
{
	for (uint16_t index = 0; index < count; ++index) {
		const uint16_t sample = samples[index];
		raw = sample;

		if (adc_filtered_raw == 0) {
			adc_filtered_raw = static_cast<uint32_t>(sample) << 6;
		}

		adc_filtered_raw += sample - (adc_filtered_raw >> 6);
	}
	adc_softirq = true;
}

void adc_softirq_handler(void)
{
	const float adc_scale = 3.26f / (4095.0f * 64.0f);
	const float voltage = static_cast<float>(adc_filtered_raw) * adc_scale;
	adc_last_val = voltage;

}

#endif /* USE_ADC */

