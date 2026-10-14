
/*
 * rtc_mode2_init.h
 *
 * Created: 1.9.2026 15.34.37
 *  Author: Kari63
 */ 
#include "samd21.h"
#include "rtc_mode2_init.h"



void rtc_mode2_init(void)
{
    
    PM->APBAMASK.reg |= PM_APBAMASK_RTC;

    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID_RTC |
    GCLK_CLKCTRL_GEN_GCLK3 |
    GCLK_CLKCTRL_CLKEN;
    while (GCLK->STATUS.bit.SYNCBUSY) { }

  
    RTC->MODE2.CTRL.reg = RTC_MODE2_CTRL_SWRST;
    while (RTC->MODE2.STATUS.bit.SYNCBUSY)   {}
  
    RTC->MODE2.CTRL.reg = RTC_MODE2_CTRL_MODE_CLOCK |
                          RTC_MODE2_CTRL_PRESCALER_DIV1024;

                        
    while (RTC->MODE2.STATUS.bit.SYNCBUSY)   {}
 

    
    uint32_t sec  = 00;
    uint32_t min  = 00;
    uint32_t hour = 00;
    uint32_t day  = 00;
    uint32_t mon  = 00;
    uint32_t year = 00;

    
    uint32_t clkval =
        (sec  << 0)  |   // SECOND:  bits 0–5
        (min  << 6)  |   // MINUTE: bits 6–11
        (hour << 12) |   // HOUR:   bits 12–16
        (day  << 17) |   // DAY:    bits 17–21
        (mon  << 22) |   // MONTH:  bits 22–25
        (year << 26);    // YEAR:   bits 26–31

    RTC->MODE2.CLOCK.reg = clkval;
    while (RTC->MODE2.STATUS.bit.SYNCBUSY) {}
  
   RTC->MODE2.Mode2Alarm[0].ALARM.reg = RTC_MODE2_ALARM_SECOND(0);
   while (RTC->MODE2.STATUS.bit.SYNCBUSY) {}

   
   RTC->MODE2.Mode2Alarm[0].MASK.reg = RTC_MODE2_MASK_SEL_SS;
   while (RTC->MODE2.STATUS.bit.SYNCBUSY) {}

   
   RTC->MODE2.INTENSET.reg = RTC_MODE2_INTENSET_ALARM0;
   while (RTC->MODE2.STATUS.bit.SYNCBUSY) {}
  
      NVIC_EnableIRQ(RTC_IRQn);

  RTC->MODE2.CTRL.reg |= RTC_MODE2_CTRL_ENABLE;
  while (RTC->MODE2.STATUS.bit.SYNCBUSY)   { }
   
  while (RTC->MODE2.STATUS.bit.SYNCBUSY)  {} 
  
}
