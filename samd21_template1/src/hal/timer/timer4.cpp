
/*
 * timer4.cpp
 *
 * Created: 31.8.2026 13.02.29
 *  Author: Kari63
 */ 
#include "project_config.h"

#if USE_TC4

#include "timer4.h"
#include "softirq.h"
#include "samd21.h"

volatile uint16_t next_top4 = 3905;
volatile uint16_t next_duty4;
 



// Weak callback (the application overrides it)
__attribute__((weak)) void timer4_onMatch() {}

 void TC4_Handler(void)
{
	if (TC4->COUNT16.INTFLAG.bit.MC0)
	{
		TC4->COUNT16.INTFLAG.reg = TC_INTFLAG_MC0;

		timer4_onMatch();
	}
}


#endif /* USE_TC4 */


