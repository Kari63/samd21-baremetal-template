/*
 * rtc_calendar2.cpp
 *
 * Created: 1.9.2026 15.47.11
 *  Author: Kari63
 */ 

#include "project_config.h"

#if USE_RTC

#include "samd21.h"
#include <stdint.h>

#include "rtc_calendar2.h"


RTC_Time_t rtc_read_time()
{
	 
	RTC_Time_t currentTime;

	// 1. Read the raw 32-bit CLOCK. RCONT (rtc_mode2_init) keeps the read
	//    buffer up to date, so no SYNCBUSY wait is needed.
	uint32_t clock_reg = RTC->MODE2.CLOCK.reg;

	// 2. Decode as in the datasheet
	currentTime.second = (uint8_t)((clock_reg >> 0)  & 0x3F);
	currentTime.minute = (uint8_t)((clock_reg >> 6)  & 0x3F);
	currentTime.hour   = (uint8_t)((clock_reg >> 12) & 0x1F);
	currentTime.day    = (uint8_t)((clock_reg >> 17) & 0x1F);
	currentTime.month  = (uint8_t)((clock_reg >> 22) & 0x0F);
	currentTime.year   = (uint8_t)((clock_reg >> 26) & 0x3F);

	return currentTime;
}

#endif /* USE_RTC */
