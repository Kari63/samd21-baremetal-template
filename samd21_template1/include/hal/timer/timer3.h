/*
 * timer3.h
 *
 * Created: 31.8.2026 13.03.07
 *  Author: Kari63
 */ 

#pragma once
#include <stdint.h>

extern volatile uint16_t next_top3;
extern volatile uint16_t next_duty3;
extern volatile bool adc_softirq;



// Software callbacks
void timer3_onMatch();
void timer3_onOverflow();