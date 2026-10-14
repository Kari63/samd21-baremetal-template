/*
 * softirq.h
 *
 * Created: 22.9.2026 0.06.01
 *  Author: Kari63
 */

#pragma once

#include "adc_read.h"
#include "timer3.h"
#include "systick.h"
#include "system_commands.h"
#include "lcd_update.h"


#define CMD_SOFTIRQ_DISPATCH()        \
do {                                  \
	if (cmd_softirq) {                \
		cmd_softirq = false;          \
		usart4_has_data = false;      \
		command_softirq();            \
	}                                 \
} while (0)

#define ADC_SOFTIRQ_DISPATCH()        \
do {                                  \
	if (adc_softirq) {                \
		adc_softirq = false;          \
		adc_softirq_handler();        \
	}                                 \
} while (0)

#define LCD_SOFTIRQ_DISPATCH()        \
do {                                  \
	if (lcd_softirq) {                \
		lcd_softirq = false;          \
		lcd_softirq_handler();        \
	}                                 \
} while (0)
