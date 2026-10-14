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

	// 1. Wait until the RTC is not synchronizing
	while (RTC->MODE2.STATUS.bit.SYNCBUSY) { }

	// 2. Read the raw 32-bit CLOCK
	uint32_t clock_reg = RTC->MODE2.CLOCK.reg;

	// 3. Decode as in the datasheet
	currentTime.second = (uint8_t)((clock_reg >> 0)  & 0x3F);
	currentTime.minute = (uint8_t)((clock_reg >> 6)  & 0x3F);
	currentTime.hour   = (uint8_t)((clock_reg >> 12) & 0x1F);
	currentTime.day    = (uint8_t)((clock_reg >> 17) & 0x1F);
	currentTime.month  = (uint8_t)((clock_reg >> 22) & 0x0F);
	currentTime.year   = (uint8_t)((clock_reg >> 26) & 0x3F);

	return currentTime;
}


void RTC_Handler(void)
{
	if (RTC->MODE2.INTFLAG.reg & RTC_MODE2_INTFLAG_ALARM0)
	{
		// 1. Acknowledge the interrupt flag
		RTC->MODE2.INTFLAG.reg = RTC_MODE2_INTFLAG_ALARM0;
		while (RTC->MODE2.STATUS.bit.SYNCBUSY) {}

		// 2. Read the old alarm value, NOT the clock (the old alarm is stable and synchronous)
		uint32_t last_alarm_second = RTC->MODE2.Mode2Alarm[0].ALARM.bit.SECOND;

		// 3. Move the alarm forward by exactly one (as in MODE0 COMP mode)
		uint32_t next_second = (last_alarm_second + 1) % 60;

		// 4. Write the new value
		RTC->MODE2.Mode2Alarm[0].ALARM.bit.SECOND = next_second;
		while (RTC->MODE2.STATUS.bit.SYNCBUSY) {}

		// 5. LCD update (the main loop reads the clock correctly with rtc_read_time)
		rtc_onSecond();
	}
}


__attribute__((weak)) void rtc_onCompMatch() {}



void rtc_onSecond(void)
{	
	rtc_onCompMatch();
	
}

#endif /* USE_RTC */
