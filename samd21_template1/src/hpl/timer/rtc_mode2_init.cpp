
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

    // No ALARM interrupt: GCLK3 = 1.024 kHz, so every synchronized write
    // takes 5-6 GCLK_RTC periods (~5-6 ms). The calendar is read by
    // polling (rtc_read_time).

    RTC->MODE2.CTRL.reg |= RTC_MODE2_CTRL_ENABLE;
    while (RTC->MODE2.STATUS.bit.SYNCBUSY) {}

    rtc_read_sync_continuous();
}

// Continuous read synchronization: CLOCK is kept up to date in the read
// buffer in the background, rtc_read_time() reads without RREQ and
// without waiting (the value is at most ~6 ms old).
void rtc_read_sync_continuous(void)
{
    RTC->MODE2.READREQ.reg = RTC_READREQ_RREQ | RTC_READREQ_RCONT |
                             RTC_READREQ_ADDR(RTC_MODE2_CLOCK_OFFSET);
    while (RTC->MODE2.STATUS.bit.SYNCBUSY) {}
}
