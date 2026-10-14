/*
 * timer5.cpp
 *
 * Created: 5.9.2026 19.10.30
 *  Author: Kari63
 */ 

#include "project_config.h"

#if USE_TC5

#include "timer5.h"
#include "samd21.h"





// Weak callback (the application overrides it)
__attribute__((weak)) void timer5_onMatch() {}

 void TC5_Handler(void)
 {
	 if (TC5->COUNT16.INTFLAG.bit.MC0)
	 {
		 TC5->COUNT16.INTFLAG.reg = TC_INTFLAG_MC0;
		
		 timer5_onMatch();
	 }
 }


#endif /* USE_TC5 */





