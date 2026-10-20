#include "samd21.h"
#include "systick.h"
#include "project_config.h"
#if USE_DHT20
#include "dht20.h"
#endif
#include <stdbool.h>

volatile uint32_t systick_ms_laskuri = 0;
volatile bool cmd_softirq = false;
volatile bool lcd_softirq =false;
volatile bool ac_softirq0 =false;

volatile bool usart3_has_data = false;




void systick_init(void)
{
	SysTick->LOAD = (48000000UL / 1000UL) - 1UL;   // every 1 ms
	SysTick->VAL  = 0UL;
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
	SysTick_CTRL_TICKINT_Msk   | // enables the interrupt
	SysTick_CTRL_ENABLE_Msk;
}




void delay_ms(uint32_t ms)
{
	uint32_t aloitus_aika = systick_ms_laskuri;
		
	while ((systick_ms_laskuri - aloitus_aika) < ms)
	{
		__asm__("nop"); 
	}
}

void SysTick_Handler(void)
{
	systick_ms_laskuri++;
#if USE_DHT20
	dht20_tick();                       // countdown -> dht20_softirq
#endif
	
	
	
	if (usart3_has_data) {
	   cmd_softirq = true;
	}
	
}
