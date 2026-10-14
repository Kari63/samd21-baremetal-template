/*
 * iinkkarit.h
 *
 * Created: 15.9.2026 8.32.42
 *  Author: Kari63
 */ 



#pragma once



// hpl



#include "debounce.h"
#include "sercom_init.h"  //prototypes
#include "timer_init.h"   //prototypes
#include "systick.h"
#include "adc_init.h"
#include "ac_init.h"

#include "board_pins.h"
#include "gpio_simplex.h"
#include "gpio_pins.h"

#include "eic_pins.h"
#include "eic_init.h"
#include "evsys.h"
#include "app_init.h"

//hal


#include "ac_control.h"

#include "i2c_0.h"
#include "pcf8574.h"
#include "rtc_calendar2.h"
#include "spi5.h"


#include "timer3.h"
#include "timer4.h"
#include "timer5.h"
#include "timer6.h"
#include "timer7.h"

#include "flash_wr.h"
#include "sst26_flash.h"

#include "usart2.h"
#include "usart4.h"
#include "dma.h"


//drivers
#include "dht20.h"
#include "lcdpcf8574.h"
#include "drv8833.h"
#include "tft.h"
#include "dma_tft.h"

#include "FreeMono9pt7b.h"
#include "FreeMono12pt7b.h"
#include "FreeSans12pt7b.h"
#include "FreeSans18pt7b.h"
#include "FreeSansBold12pt7b.h"
#include "FreeSansBold18pt7b.h"


//core
#include "formatter.h"
#include "backend.h"
#include "system_commands.h"
#include "systeminit.h"
#include "softirq.h"

//math
#include "random.h"

//api
#include "dyn_flash_formatter_write.h"
#include "lcd_update.h"

//app
#include "app_init.h"
#include "app_run.h"




