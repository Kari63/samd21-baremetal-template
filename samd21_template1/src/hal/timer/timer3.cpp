/*
 * timer3.cpp
 *
 * Created: 31.8.2026 13.02.29
 *  Author: Kari63
 */ 
#include "project_config.h"

#if USE_TC3

#include "timer3.h"
#include "softirq.h"
#include "samd21.h"

volatile uint16_t next_top3 =8000;
volatile uint16_t next_duty3;
volatile bool adc_softirq; 
extern volatile uint16_t raw ;      



__attribute__((weak)) void timer3_onMatch() {}

 void TC3_Handler(void)
{
	if (TC3->COUNT16.INTFLAG.bit.MC0)
	{
		TC3->COUNT16.INTFLAG.reg = TC_INTFLAG_MC0;

		
		timer3_onMatch();
	}
}


#endif /* USE_TC3 */


