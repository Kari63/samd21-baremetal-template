/**
 * Initialize the system
 *
 * @brief  Setup the microcontroller system.
 *         Initialize the System and update the SystemCoreClock variable.
 */

#include <stdint.h>
#include "samd21.h"

uint32_t SystemCoreClock = 48000000;

void SystemInit(void)
{
    /* 1) Flash wait-state: 1 WS @ 48 MHz */
    NVMCTRL->CTRLB.bit.RWS = 1;

    /* 2) Enable the external 32.768 kHz crystal (XOSC32K) */
    SYSCTRL->XOSC32K.reg =
        SYSCTRL_XOSC32K_XTALEN    |  /* use a crystal */
        SYSCTRL_XOSC32K_EN32K     |  /* 32 kHz output to GCLK */
        SYSCTRL_XOSC32K_RUNSTDBY  |  /* may run in standby */
        SYSCTRL_XOSC32K_STARTUP(6);  /* sufficient start-up time */

    /* ONDEMAND and AAMPEN off so that the start-up does not hang */
    SYSCTRL->XOSC32K.bit.ONDEMAND = 0;
    SYSCTRL->XOSC32K.bit.AAMPEN   = 0;

    SYSCTRL->XOSC32K.bit.ENABLE = 1;
    while (!SYSCTRL->PCLKSR.bit.XOSC32KRDY);

    /* 3) GCLK1 = XOSC32K (DFLL reference) */
    GCLK->GENDIV.reg =
        GCLK_GENDIV_ID(1) |
        GCLK_GENDIV_DIV(0);

    GCLK->GENCTRL.reg =
        GCLK_GENCTRL_ID(1)        |
        GCLK_GENCTRL_SRC_XOSC32K  |
        GCLK_GENCTRL_GENEN;

    while (GCLK->STATUS.bit.SYNCBUSY);

    /* 3.5) Route GCLK1 to the DFLL48M as its reference clock */
    GCLK->CLKCTRL.reg =
        GCLK_CLKCTRL_ID(SYSCTRL_GCLK_ID_DFLL48) |
        GCLK_CLKCTRL_GEN_GCLK1                  |
        GCLK_CLKCTRL_CLKEN;

    /* 4) DFLL48M init - open loop -> closed loop */
    while (!SYSCTRL->PCLKSR.bit.DFLLRDY);
    SYSCTRL->DFLLCTRL.reg = 0;
    while (!SYSCTRL->PCLKSR.bit.DFLLRDY);

    /* 4b) Factory calibration from NVM */
    uint32_t coarse =
        (*((uint32_t *)FUSES_DFLL48M_COARSE_CAL_ADDR) &
         FUSES_DFLL48M_COARSE_CAL_Msk) >> FUSES_DFLL48M_COARSE_CAL_Pos;

    uint32_t fine =
        (*((uint32_t *)FUSES_DFLL48M_FINE_CAL_ADDR) &
         FUSES_DFLL48M_FINE_CAL_Msk) >> FUSES_DFLL48M_FINE_CAL_Pos;

    SYSCTRL->DFLLVAL.reg =
        SYSCTRL_DFLLVAL_COARSE(coarse) |
        SYSCTRL_DFLLVAL_FINE(fine);

    while (!SYSCTRL->PCLKSR.bit.DFLLRDY);

    /* 4c) Multiplier and steps */
    SYSCTRL->DFLLMUL.reg =
        SYSCTRL_DFLLMUL_MUL(1500) |  /* 48 MHz / 32 kHz = 1500 */
        SYSCTRL_DFLLMUL_FSTEP(10) |
        SYSCTRL_DFLLMUL_CSTEP(31);

    while (!SYSCTRL->PCLKSR.bit.DFLLRDY);

    /* 4d) DFLL on (open loop) */
    SYSCTRL->DFLLCTRL.reg = SYSCTRL_DFLLCTRL_ENABLE;
    while (!SYSCTRL->PCLKSR.bit.DFLLRDY);

    /* 4e) Closed-loop + STABLE */
    SYSCTRL->DFLLCTRL.reg =
        SYSCTRL_DFLLCTRL_ENABLE |
        SYSCTRL_DFLLCTRL_MODE   |
        SYSCTRL_DFLLCTRL_STABLE;

    while (!SYSCTRL->PCLKSR.bit.DFLLRDY);

    /* Wait for lock */
    while (!SYSCTRL->PCLKSR.bit.DFLLLCKC &&
           !SYSCTRL->PCLKSR.bit.DFLLLCKF);

    /* 5) GCLK0 = DFLL48M (CPU clock) */
    GCLK->GENDIV.reg =
        GCLK_GENDIV_ID(0) |
        GCLK_GENDIV_DIV(0);

    GCLK->GENCTRL.reg =
        GCLK_GENCTRL_ID(0)        |
        GCLK_GENCTRL_SRC_DFLL48M  |
        GCLK_GENCTRL_GENEN;

    while (GCLK->STATUS.bit.SYNCBUSY);

    /* 6) CPU and APB prescalers = /1 */
    PM->CPUSEL.reg  = PM_CPUSEL_CPUDIV_DIV1;
    PM->APBASEL.reg = PM_APBASEL_APBADIV_DIV1;
    PM->APBBSEL.reg = PM_APBBSEL_APBBDIV_DIV1;
    PM->APBCSEL.reg = PM_APBCSEL_APBCDIV_DIV1;
	
    /* GCLK2 = OSC8M / 1 */
	GCLK->GENDIV.reg =
	GCLK_GENDIV_ID(3) |
	GCLK_GENDIV_DIV(32);   

	GCLK->GENCTRL.reg =
	GCLK_GENCTRL_ID(3) |
	GCLK_GENCTRL_SRC_XOSC32K |
	GCLK_GENCTRL_GENEN; 
	
	
	/* 1) Make sure OSC8M runs at the full 8 MHz without division */
	SYSCTRL->OSC8M.bit.PRESC = SYSCTRL_OSC8M_PRESC_0_Val; // prescaler = /1 (full 8 MHz)
	SYSCTRL->OSC8M.bit.ONDEMAND = 0;                      // do not stop the clock automatically
	SYSCTRL->OSC8M.bit.RUNSTDBY = 1;                      // runs in standby too
	SYSCTRL->OSC8M.bit.ENABLE = 1;
	while(!SYSCTRL->PCLKSR.bit.OSC8MRDY);                 // wait until 8 MHz is stable

	/* 2) GCLK2 = OSC8M / 1 */
	GCLK->GENDIV.reg =
	GCLK_GENDIV_ID(2) |
	GCLK_GENDIV_DIV(0);

	GCLK->GENCTRL.reg =
	GCLK_GENCTRL_ID(2)        |
	GCLK_GENCTRL_SRC_OSC8M    |
	GCLK_GENCTRL_GENEN;

    while (GCLK->STATUS.bit.SYNCBUSY);

    /* GCLK4 = DFLL48M / 6 = 8 MHz, for accurate TC6/TC7 timing. */
    GCLK->GENDIV.reg =
    GCLK_GENDIV_ID(4) |
    GCLK_GENDIV_DIV(6);

    GCLK->GENCTRL.reg =
    GCLK_GENCTRL_ID(4)        |
    GCLK_GENCTRL_SRC_DFLL48M  |
    GCLK_GENCTRL_GENEN;

    while (GCLK->STATUS.bit.SYNCBUSY);

	 

    SystemCoreClock = 48000000;
}
