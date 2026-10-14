/**
 * project_config.h - central configuration of the SAMD21J18A bare-metal template
 *
 * One place for: feature switches, clock constants, debug options and the
 * example selector. Framework code includes this file; application code may
 * too. Nothing here is changed at runtime - everything is decided at build time.
 *
 * Values marked "CHECK" must match what systeminit() actually configures.
 * If you change a clock in systeminit(), change it here too.
 */

#pragma once

/* ------------------------------------------------------------------------- */
/* Template identity (printed in the boot banner)                            */
/* ------------------------------------------------------------------------- */
#define TEMPLATE_NAME            "samd21j18_template"
#define TEMPLATE_VERSION_MAJOR   0
#define TEMPLATE_VERSION_MINOR   9
#define TEMPLATE_VERSION_PATCH   0
#define TEMPLATE_VERSION_STRING  "0.9.0"

/* ------------------------------------------------------------------------- */
/* Feature switches: 1 = initialize and use the module, 0 = leave it idle.  */
/* The generated Makefile still compiles all project sources.                */
/* ------------------------------------------------------------------------- */

/* Core services */
#define USE_SYSTICK_DELAY    1   /* delay_ms() on SysTick                      */
#define USE_SCHEDULER        1   /* cooperative "RTOS seed" (polled SW-IRQs)   */
#define USE_RTC              1   /* RTC mode 0/2                               */

/* Communication */
#define USE_UART2            1   /* SERCOM2 USART mode						   */
#define USE_UART4            1   /* SERCOM4 USART mode                         */         
#define USE_SPI              1   /* SERCOM5 SPI mode for external flash        */
#define USE_I2C              1   /* SERCOM0 I2C mode                           */

/* Timers / events / DMA */
#define USE_TC3              1   /* ADC sampling timer                          */
#define USE_TC4              1   /* LCD refresh and TC4 event generator         */
#define USE_TC5              1   /* TC4 event consumer                           */
#define USE_TC6              1   /* software-event capture                       */
#define USE_TC7              1   /* EIC/EVSYS pulse-width capture                */
#define USE_TCC0             1   /* PWM / timing experiment                      */
#define USE_TC               (USE_TC3 || USE_TC4 || USE_TC5 || USE_TC6 || USE_TC7)
#define USE_TCC              USE_TCC0
#define USE_EIC              1   /* external interrupts (EXTINT)               */
#define USE_EVSYS            1   /* event system                               */
#define USE_DMA              1   /* DMAC                                       */

/* Analog */
#define USE_ADC              1
#define USE_AC               0   /* analog comparators                         */
#define USE_DAC              0   /* NOT IMPLEMENTED YET                        */
#define USE_I2S              0   /* NOT IMPLEMENTED YET                        */

/* Drivers / board peripherals */
#define USE_QSPI_FLASH       0   /* SST26VF/WF - read the SST26 notes in README.md first! */
#define USE_TFT              1   /* ST7735 128x160 on SERCOM3 SPI               */
#define USE_TFT_DMA          1   /* DMA accelerated TFT fills                    */
#define USE_LCD              1   /* HD44780 via PCF8574 on I2C                   */
#define USE_DHT20            0   /* DHT20 temperature/humidity sensor            */
#define USE_DRV8833          1   /* DRV8833 motor driver on TCC0                  */



/* ------------------------------------------------------------------------- */
/* Debug / diagnostics                                                       */
/* ------------------------------------------------------------------------- */
#define DEBUG_PRINT           1   /* printf-style output over debug UART      */
#define PRINT_BOOT_BANNER     1   /* name, version, build date at startup     */
#define PRINT_RESET_CAUSE     1   /* decode RCAUSE at startup                 */
#define HARDFAULT_DUMP        1   /* print PC/LR/SP/xPSR on HardFault         */
#define WDT_ENABLE            0   /* watchdog off by default                  */
#define DEBUG_PINS            0   /* toggle GPIO pins for scope timing        */

#if (PRINT_BOOT_BANNER || PRINT_RESET_CAUSE || HARDFAULT_DUMP) && !(DEBUG_PRINT && USE_UART4)
#error "Boot/fault diagnostics need DEBUG_PRINT and USE_UART4"
#endif

/* ------------------------------------------------------------------------- */
/* Example selector: exactly one example is built, or none                   */
/* The selected example provides example_init() and example_run().           */
/* It uses only the public API of the framework.                             */
/* ------------------------------------------------------------------------- */
#define EXAMPLE_NONE                  0
#define EXAMPLE_TFT_STRESS            1
#define EXAMPLE_DMA_TFT               2
#define EXAMPLE_EVSYS_TC_CAPTURE      3
#define EXAMPLE_EIC_EVSYS_TC7_PULSE   4
#define EXAMPLE_DMA_RAM                5
#define EXAMPLE_QSPI_FLASH_TEST        6
#define EXAMPLE_QSPI_DYN_PACKER        7


#define EXAMPLE_SELECT     0          //  EXAMPLE_TFT_STRESS



#if EXAMPLE_SELECT == EXAMPLE_TFT_STRESS && !(USE_TFT_DMA && USE_SYSTICK_DELAY)
#error "EXAMPLE_TFT_STRESS requires USE_TFT_DMA and USE_SYSTICK_DELAY"
#endif
#if EXAMPLE_SELECT == EXAMPLE_DMA_TFT && !USE_TFT_DMA
#error "EXAMPLE_DMA_TFT requires USE_TFT_DMA"
#endif
#if EXAMPLE_SELECT == EXAMPLE_EVSYS_TC_CAPTURE && !(USE_EVSYS && USE_TC6)
#error "EXAMPLE_EVSYS_TC_CAPTURE requires USE_EVSYS and USE_TC6"
#endif
#if EXAMPLE_SELECT == EXAMPLE_EIC_EVSYS_TC7_PULSE && !(USE_EIC && USE_EVSYS && USE_TC7)
#error "EXAMPLE_EIC_EVSYS_TC7_PULSE requires USE_EIC, USE_EVSYS and USE_TC7"
#endif
#if EXAMPLE_SELECT == EXAMPLE_DMA_RAM && !(USE_DMA && USE_TC4 && USE_TC5 && USE_EVSYS)
#error "EXAMPLE_DMA_RAM requires USE_DMA, USE_TC4, USE_TC5 and EVSYS"
#endif
#if EXAMPLE_SELECT == EXAMPLE_QSPI_FLASH_TEST && !(USE_QSPI_FLASH && USE_SYSTICK_DELAY)
#error "EXAMPLE_QSPI_FLASH_TEST requires USE_QSPI_FLASH and USE_SYSTICK_DELAY"
#endif
#if EXAMPLE_SELECT == EXAMPLE_QSPI_DYN_PACKER && !USE_QSPI_FLASH
#error "EXAMPLE_QSPI_DYN_PACKER requires USE_QSPI_FLASH"
#endif

/* ------------------------------------------------------------------------- */
/* Dependency checks - fail at build time, not at 2 a.m. on the bench        */
/* ------------------------------------------------------------------------- */

#if USE_QSPI_FLASH && !(USE_SPI)
#error "USE_QSPI_FLASH  requires USE_SPI"
#endif

#if USE_TFT_DMA && !(USE_TFT && USE_DMA)
#error "USE_TFT_DMA requires USE_TFT and USE_DMA"
#endif

#if USE_LCD && !(USE_I2C && USE_TC4 && USE_TC6)
#error "USE_LCD requires USE_I2C, USE_TC4 and USE_TC6 in this test configuration"
#endif

#if USE_TC3 && !USE_ADC
#error "USE_TC3 requires USE_ADC"
#endif
#if USE_TC5 && !USE_EVSYS
#error "USE_TC5 requires USE_EVSYS"
#endif
#if USE_TC6 && !USE_EVSYS
#error "USE_TC6 requires USE_EVSYS"
#endif
#if USE_TC7 && !(USE_EIC && USE_EVSYS)
#error "USE_TC7 requires USE_EIC and USE_EVSYS"
#endif
#if USE_DHT20 && !USE_I2C
#error "USE_DHT20 requires USE_I2C"
#endif
#if USE_DRV8833 && !USE_TCC0
#error "USE_DRV8833 requires USE_TCC0"
#endif
#if USE_EVSYS && !(USE_TC || USE_EIC || USE_DMA)
#error "USE_EVSYS has no event generators/users enabled"
#endif
#if USE_DAC
#error "DAC driver is not implemented yet (see README module table)"
#endif
#if USE_I2S
#error "I2S driver is not implemented yet (see README module table)"
#endif
