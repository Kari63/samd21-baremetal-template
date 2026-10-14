/*
 * systeminit.cpp
 *
 * Created: 4.9.2026 2.53.03
 *  Author: Kari63
 */ 
#include "project_config.h"
#include "core/systeminit.h"
#include "lcdpcf8574.h"

void systeminit() {

#if USE_SYSTICK_DELAY
systick_init();
#endif

#if USE_DMA
dmac_init();
#endif

#if USE_EIC
eic_init();
#endif

#if USE_I2C
sercom0_init();
#endif

#if USE_UART2
sercom2_init();
#endif

#if USE_UART4
sercom4_init();
#endif

#if USE_SPI
sercom5_init();
#endif

#if USE_TFT
sercom3_init();
#endif

#if USE_ADC
adc_init();
#endif

#if USE_AC
ac_init();
#endif

#if USE_TCC0
tcc0_init();
#endif
#if USE_TC3
tc3_init();
#endif
#if USE_TC4
tc4_init();
#endif
#if USE_TC5
tc5_init();
#endif
#if USE_TC6
tc6_init();
#endif
#if USE_TC7
tc7_init();
#endif

#if USE_EVSYS
evsys_init();
#endif

#if USE_RTC
rtc_mode2_init();
#endif

#if USE_LCD
 lcd_init(LCD_DISP_ON);
 lcd_led(1);
 lcd_clrscr();
#endif

#if USE_TFT
TFT_displayInit();
#endif

board_gpio_init();

}
