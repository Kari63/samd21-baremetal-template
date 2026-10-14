/**
 * \file
 *
 * \brief gcc startup file for SAMD21
 *
 * Copyright (c) 2018 Microchip Technology Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "samd21.h"

/* Memory region symbols from the linker script */
extern uint32_t _sfixed;
extern uint32_t _efixed;
extern uint32_t _etext;
extern uint32_t _srelocate;
extern uint32_t _erelocate;
extern uint32_t _szero;
extern uint32_t _ezero;
extern uint32_t _sstack;
extern uint32_t _estack;

int main(void);
void __libc_init_array(void);
void SystemInit(void);
void Reset_Handler(void);

volatile uint32_t hardfault_stacked_sp;
volatile uint32_t hardfault_stacked_lr;
volatile uint32_t hardfault_stacked_pc;
volatile uint32_t hardfault_stacked_xpsr;
volatile uint32_t hardfault_icsr;

void hardfault_capture(uint32_t *stacked) __attribute__((noreturn));
void hardfault_capture(uint32_t *stacked)
{
        hardfault_stacked_sp = (uint32_t)stacked;
        hardfault_stacked_lr = stacked[5];
        hardfault_stacked_pc = stacked[6];
        hardfault_stacked_xpsr = stacked[7];
        hardfault_icsr = SCB->ICSR;

        while (1) {
        }
}

void HardFault_Handler(void) __attribute__((naked));
void HardFault_Handler(void)
{
        __asm volatile (
                "mrs r0, msp\n"
                "b hardfault_capture\n"
        );
}

/* Default interrupt handler */
void Dummy_Handler(void)
{
    while (1) {
        /* Stay here if an undefined interrupt fires */
    }
}

/* Cortex-M0+ core exceptions */
void NonMaskableInt_Handler  ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void SVCall_Handler          ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void PendSV_Handler          ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void SysTick_Handler         ( void ) __attribute__ ((weak, alias("Dummy_Handler")));

/* Peripheral interrupts */
void PM_Handler              ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void SYSCTRL_Handler         ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void WDT_Handler             ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void RTC_Handler             ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void EIC_Handler             ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void NVMCTRL_Handler         ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void DMAC_Handler            ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
#ifdef ID_USB
void USB_Handler             ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
#endif
void EVSYS_Handler           ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void SERCOM0_Handler         ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void SERCOM1_Handler         ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void SERCOM2_Handler         ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void SERCOM3_Handler         ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
#ifdef ID_SERCOM4
void SERCOM4_Handler         ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
#endif
#ifdef ID_SERCOM5
void SERCOM5_Handler         ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
#endif
void TCC0_Handler            ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void TCC1_Handler            ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void TCC2_Handler            ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void TC3_Handler             ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void TC4_Handler             ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void TC5_Handler             ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
#ifdef ID_TC6
void TC6_Handler             ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
#endif
#ifdef ID_TC7
void TC7_Handler             ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
#endif
#ifdef ID_ADC
void ADC_Handler             ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
#endif
#ifdef ID_AC
void AC_Handler              ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
#endif
#ifdef ID_DAC
void DAC_Handler             ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
#endif
#ifdef ID_PTC
void PTC_Handler             ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
#endif
void I2S_Handler             ( void ) __attribute__ ((weak, alias("Dummy_Handler")));

/* Vector table at the start of flash (.vectors section) */
__attribute__ ((section(".vectors")))
const DeviceVectors exception_table = {

        /* Initial stack pointer from the linker script */
        .pvStack                = (void*) (&_estack),

        .pfnReset_Handler       = (void*) Reset_Handler,
        .pfnNonMaskableInt_Handler = (void*) NonMaskableInt_Handler,
        .pfnHardFault_Handler   = (void*) HardFault_Handler,
        .pvReservedM12          = (void*) (0UL),
        .pvReservedM11          = (void*) (0UL),
        .pvReservedM10          = (void*) (0UL),
        .pvReservedM9           = (void*) (0UL),
        .pvReservedM8           = (void*) (0UL),
        .pvReservedM7           = (void*) (0UL),
        .pvReservedM6           = (void*) (0UL),
        .pfnSVCall_Handler      = (void*) SVCall_Handler,
        .pvReservedM4           = (void*) (0UL),
        .pvReservedM3           = (void*) (0UL),
        .pfnPendSV_Handler      = (void*) PendSV_Handler,
        .pfnSysTick_Handler     = (void*) SysTick_Handler,

        /* Peripheral interrupts */
        .pfnPM_Handler          = (void*) PM_Handler,             /*  0 Power Manager */
        .pfnSYSCTRL_Handler     = (void*) SYSCTRL_Handler,        /*  1 System Control */
        .pfnWDT_Handler         = (void*) WDT_Handler,            /*  2 Watchdog Timer */
        .pfnRTC_Handler         = (void*) RTC_Handler,            /*  3 Real-Time Counter */
        .pfnEIC_Handler         = (void*) EIC_Handler,            /*  4 External Interrupt Controller */
        .pfnNVMCTRL_Handler     = (void*) NVMCTRL_Handler,        /*  5 Non-Volatile Memory Controller */
        .pfnDMAC_Handler        = (void*) DMAC_Handler,           /*  6 Direct Memory Access Controller */
#ifdef ID_USB
        .pfnUSB_Handler         = (void*) USB_Handler,            /*  7 Universal Serial Bus */
#else
        .pvReserved7            = (void*) (0UL),                  /*  7 Reserved */
#endif
        .pfnEVSYS_Handler       = (void*) EVSYS_Handler,          /*  8 Event System Interface */
        .pfnSERCOM0_Handler     = (void*) SERCOM0_Handler,        /*  9 Serial Communication Interface 0 */
        .pfnSERCOM1_Handler     = (void*) SERCOM1_Handler,        /* 10 Serial Communication Interface 1 */
        .pfnSERCOM2_Handler     = (void*) SERCOM2_Handler,        /* 11 Serial Communication Interface 2 */
        .pfnSERCOM3_Handler     = (void*) SERCOM3_Handler,        /* 12 Serial Communication Interface 3 */
#ifdef ID_SERCOM4
        .pfnSERCOM4_Handler     = (void*) SERCOM4_Handler,        /* 13 Serial Communication Interface 4 */
#else
        .pvReserved13           = (void*) (0UL),                  /* 13 Reserved */
#endif
#ifdef ID_SERCOM5
        .pfnSERCOM5_Handler     = (void*) SERCOM5_Handler,        /* 14 Serial Communication Interface 5 */
#else
        .pvReserved14           = (void*) (0UL),                  /* 14 Reserved */
#endif
        .pfnTCC0_Handler        = (void*) TCC0_Handler,           /* 15 Timer Counter Control 0 */
        .pfnTCC1_Handler        = (void*) TCC1_Handler,           /* 16 Timer Counter Control 1 */
        .pfnTCC2_Handler        = (void*) TCC2_Handler,           /* 17 Timer Counter Control 2 */
        .pfnTC3_Handler         = (void*) TC3_Handler,            /* 18 Basic Timer Counter 0 */
        .pfnTC4_Handler         = (void*) TC4_Handler,            /* 19 Basic Timer Counter 1 */
        .pfnTC5_Handler         = (void*) TC5_Handler,            /* 20 Basic Timer Counter 2 */
#ifdef ID_TC6
        .pfnTC6_Handler         = (void*) TC6_Handler,            /* 21 Basic Timer Counter 3 */
#else
        .pvReserved21           = (void*) (0UL),                  /* 21 Reserved */
#endif
#ifdef ID_TC7
        .pfnTC7_Handler         = (void*) TC7_Handler,            /* 22 Basic Timer Counter 4 */
#else
        .pvReserved22           = (void*) (0UL),                  /* 22 Reserved */
#endif
#ifdef ID_ADC
        .pfnADC_Handler         = (void*) ADC_Handler,            /* 23 Analog Digital Converter */
#else
        .pvReserved23           = (void*) (0UL),                  /* 23 Reserved */
#endif
#ifdef ID_AC
        .pfnAC_Handler          = (void*) AC_Handler,             /* 24 Analog Comparators */
#else
        .pvReserved24           = (void*) (0UL),                  /* 24 Reserved */
#endif
#ifdef ID_DAC
        .pfnDAC_Handler         = (void*) DAC_Handler,            /* 25 Digital Analog Converter */
#else
        .pvReserved25           = (void*) (0UL),                  /* 25 Reserved */
#endif
#ifdef ID_PTC
        .pfnPTC_Handler         = (void*) PTC_Handler,            /* 26 Peripheral Touch Controller */
#else
        .pvReserved26           = (void*) (0UL),                  /* 26 Reserved */
#endif
        .pfnI2S_Handler         = (void*) I2S_Handler             /* 27 Inter-IC Sound Interface */
};

/**
 * @brief Microcontroller start-up sequence (Reset Handler)
 */
void Reset_Handler(void)
{
    uint32_t *pSrc, *pDest;

    /* 1) Set the vector table offset (VTOR) first.
          This makes sure interrupts are routed correctly from the start. */
    SCB->VTOR = ((uint32_t)&exception_table & SCB_VTOR_TBLOFF_Msk);

    /* 2) Clock init (48 MHz DFLL closed loop) before the RAM copy.
          This makes the following memory operations considerably faster. */
    SystemInit();

    /* 3) Copy initialized globals from flash to RAM (.data section) */
    pSrc  = &_etext;
    pDest = &_srelocate;

    if (pSrc != pDest) {
        while (pDest < &_erelocate) {
            *pDest++ = *pSrc++;
        }
    }

    /* 4) Zero the uninitialized globals in RAM (.bss section) */
    pDest = &_szero;
    while (pDest < &_ezero) {
        *pDest++ = 0;
    }

    /* 5) Initialize the C/C++ runtime tables (constructors) */
    __libc_init_array();

    /* 6) Jump to the application */
    main();

    /* If main ever returns, stay in an endless loop */
    while (1);
}
