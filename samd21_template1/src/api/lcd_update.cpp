


#include "project_config.h"

#if USE_LCD

#include "samd21.h"
#include "lcdpcf8574.h"
#include "formatter.h"
#include "adc_read.h"
#include "rtc_calendar2.h"
#include "timer5.h"
#include "timer6.h"
#include "timer7.h"
#include <stdint.h>

// Tick time in microseconds = PRESCALER / 8 (8 MHz GCLK2)
// DIV256 -> 32, DIV64 -> 8, DIV16 -> 2, DIV8 -> 1
#define TC6_TICK_US  1UL

// Full refresh on every Nth LCD_update. TC4 period 2000 x 128 us = 256 ms
// (next_top4 = 2000) -> 40 x 256 ms ~ 10 s.
#define LCD_FULL_REFRESH_UPDATES  40

uint16_t tc5_read_count(void);

void LCD_update()
{
	uint16_t min_ticks;
	uint16_t max_ticks;
	tc6_minmax_take(min_ticks, max_ticks);

	uint32_t min_us = min_ticks == 0xFFFF ? 0 : ((uint32_t)min_ticks * TC6_TICK_US);
	uint32_t max_us = (uint32_t)max_ticks * TC6_TICK_US;

	lcd_clrscr();		// buffer only, cursor (0,0)

	
	#if EXAMPLE_SELECT == EXAMPLE_EVSYS_TC_CAPTURE
	print("MIN=%u us\nMAX=%u us", min_us, max_us, 2);
	#endif
	
	#if EXAMPLE_SELECT ==  EXAMPLE_EIC_EVSYS_TC7_PULSE
	print("TC7 pw=%u mS\nF=%u Hz" , tc7pw ,f_x100 ,2 );
	#endif
	
	
	#if EXAMPLE_SELECT == EXAMPLE_NONE
	RTC_Time_t current_time = rtc_read_time();
	print("%02u:%02u:%02u\n", (uint32_t)current_time.hour,
		(uint32_t)current_time.minute, (uint32_t)current_time.second, 2);
	print("ADC=%03f\n", adc_last_val, 2);
	#endif
	
	//	print("%02x %02x %02x\n", sst26_mid2,sst26_d02,sst26_d12, 2);
	//	print("1:%02X 2:%02X 3:%02X\n", sst26_sr1, sst26_sr2, sst26_rdcr, 2);
	//	print("%02X %02X %02X %02X\n", sst26_prog_rx[0],sst26_prog_rx[1],sst26_prog_rx[2],sst26_prog_rx[3],2);

	// Periodic full refresh: repairs characters corrupted by a disturbance
	// that the difference check cannot see. ~5 ms once per LCD_FULL_REFRESH_UPDATES rounds.
	static uint8_t refresh_count;
	if (++refresh_count >= LCD_FULL_REFRESH_UPDATES) {
		refresh_count = 0;
		lcd_invalidate();
	}

	lcd_flush();		// only the changed characters to the display
}

// The TC4 interrupt sets the lcd_softirq flag; the update runs in the main loop.
// The ignore flag is set right before the update, so that the next TC6 capture
// (same iteration) is excluded from the loop-time min/max measurement.
void lcd_softirq_handler(void)
{
#if USE_TC6
	tc6_ignore_next_interval();
#endif
	LCD_update();
}

#endif