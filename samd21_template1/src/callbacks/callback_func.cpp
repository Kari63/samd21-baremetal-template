/*
 * callback_func.cpp
 *
 * Created: 11.9.2026 0.17.26
 *  Author: Kari63
 */


#include "project_config.h"

#include "callbacks.h"
#include "softirq.h"

void timer4_onMatch()
{
#if USE_TC4
	TC4->COUNT16.CC[0].reg = next_top4;
	gpio_toggle(LED);
	#if USE_LCD
	lcd_softirq = true;
	#endif
#endif
}

void timer5_onMatch()
{
// gpio_toggle(LED);      
	
}

void timer6_onMatch()
{	

}

void timer7_onMatch()
{
#if USE_TC7

//	gpio_toggle(LED);
#endif
}


void ac_onComp0Match()
{	
	//gpio_toggle(LED);
}

void ac_onComp1Match()
{	 
	// gpio_toggle(LED);
}

void rtc_onCompMatch()
{
	
}