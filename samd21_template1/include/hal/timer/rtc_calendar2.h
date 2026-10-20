
/*
 * rtc_calendar2_h.S
 *
 * Created: 1.9.2026 15.43.07
 *  Author: Kari63
 */ 


#pragma once

#include <stdint.h>
//extern volatile uint16_t adc_last_val;

typedef struct
{
	uint8_t second;
	uint8_t minute;
	uint8_t hour;
	uint8_t day;
	uint8_t month;
	uint8_t year;
} RTC_Time_t;

RTC_Time_t rtc_read_time(void);
void LCD_update(void);



