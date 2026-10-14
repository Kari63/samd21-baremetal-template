/*
 * ac_control.cpp
 *
 * Created: 9.9.2026
 *  Author: Kari63
 */ 

#include "project_config.h"

#if USE_AC

#include "ac_control.h"
#include "samd21.h"


static inline void ac_ctrl_sync(void)
{
	while (AC->STATUSB.reg & AC_STATUSB_SYNCBUSY);
}



// === 2. RUNTIME THRESHOLD ADJUSTMENT (0 - 63) ===
void ac_set_threshold(uint8_t comp_inst, uint8_t value)
{
	if (comp_inst > 1) return;
	if (value > 63) value = 63;

	
	AC->SCALER[comp_inst].reg = AC_SCALER_VALUE(value);
	ac_ctrl_sync();
}


__attribute__((weak)) void ac_onComp0Match( ) {}
__attribute__((weak)) void ac_onComp1Match( ) {}
__attribute__((weak)) void ac_onWindowMatch( ) {}


void AC_Handler(void)
{
	uint8_t flags = AC->INTFLAG.reg;

	// === COMMAND 0: COMP0 HANDLING ===
	if (flags & AC_INTFLAG_COMP0)
	{
		REG_AC_INTFLAG = AC_INTFLAG_COMP0;
			
		ac_onComp0Match();
		
		
				
	}
	
	// === COMMAND 1: COMP1 HANDLING ===
	if (flags & AC_INTFLAG_COMP1)
	{
		AC->INTFLAG.reg = AC_INTFLAG_COMP1;
		
		ac_onComp1Match();
		
		while (REG_AC_INTFLAG & AC_INTFLAG_COMP0)
		{
			REG_AC_INTFLAG = AC_INTFLAG_COMP0;
		}
		
			
	}

	// === COMMAND 2: WINDOW HANDLING ===
	if (flags & AC_INTFLAG_WIN0)
	{
		AC->INTFLAG.reg = AC_INTFLAG_WIN0;
		while (AC->STATUSB.reg & AC_STATUSB_SYNCBUSY);
		AC->INTFLAG.reg = AC_INTFLAG_WIN0;

		ac_onWindowMatch();
		
		while (REG_AC_INTFLAG & AC_INTFLAG_COMP0)
		{
			REG_AC_INTFLAG = AC_INTFLAG_COMP0;
		}
	}
}

#endif /* USE_AC */



