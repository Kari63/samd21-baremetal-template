/*
 * main.cpp
 *
 * Created: 1.10.2026 22.50.00
 *  Author: Kari63
 */

#include <samd21.h>
//#include "iinkkarit.h"
#include "timer4.h"
#include "project_config.h"
#include "system_commands.h"
#include "systeminit.h"
#include "softirq.h"
#include "examples.h"
#include "app_init.h"
#include "app_run.h"

#include "dma_tft.h"
#include "dma.h"
#include "systick.h"




int main(void)
{
	
	systeminit();
	next_top4 = 2000;
	// TCC0->PER.reg = 8000;
	
	app_init();

#if EXAMPLE_SELECT != EXAMPLE_NONE
	example_init();
#endif
	
	__enable_irq();
	command_console_banner();


	while (1)
	{
				
		CMD_SOFTIRQ_DISPATCH();
		ADC_SOFTIRQ_DISPATCH();
#if USE_LCD
		LCD_SOFTIRQ_DISPATCH();
#endif
	
#if EXAMPLE_SELECT != EXAMPLE_NONE
		example_run();
#endif
		app_run();
		
	
		
		
	}
}
