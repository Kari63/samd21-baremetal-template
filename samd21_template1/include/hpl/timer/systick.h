/*
 * systick.h
 *
 * Created: 24.8.2026 22.40.30
 *  Author: Kari63
 */ 


#pragma once
#include <stdint.h>
#include <stdbool.h>

extern volatile uint32_t systick_ms_laskuri;
extern volatile bool cmd_softirq;
extern volatile bool usart4_has_data;
extern volatile bool lcd_softirq;


void systick_init(void);
void delay_ms(uint32_t ms);