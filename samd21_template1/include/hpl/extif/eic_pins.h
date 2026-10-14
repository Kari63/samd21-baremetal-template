/*
 * eic_pins.h
 *
 * EIC line configuration from a GpioPin.
 *
 * Usage:
 *   static const GpioPin BTN = PA(15);
 *   gpio_init_input_pullup(BTN);                       // 1. pin as input + pull-up (GPIO layer)
 *   eic_pin_config(BTN, EXTINT_FALL, EIC_PIN_IRQ, cb); // 2. PMUX (function A) + EIC line
 *
 * The order 1 -> 2 is mandatory: gpio_init_input_pullup writes the whole
 * PINCFG register and would clear the PMUXEN bit.
 *
 * Requires: eic_init() called first.
 * EIC_Handler is in this driver (eic_pins.cpp); do not define your own.
 *
 * Note pins: PA08 = NMI (no EXTINT), PA30/PA31 = SWD (debug), PA24/PA25 = USB.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "samd21.h"
#include "gpio_simplex.h"

/* SENSE values straight from the CMSIS constants */
#define EXTINT_NONE   EIC_CONFIG_SENSE0_NONE_Val
#define EXTINT_RISE   EIC_CONFIG_SENSE0_RISE_Val
#define EXTINT_FALL   EIC_CONFIG_SENSE0_FALL_Val
#define EXTINT_BOTH   EIC_CONFIG_SENSE0_BOTH_Val
#define EXTINT_HIGH   EIC_CONFIG_SENSE0_HIGH_Val
#define EXTINT_LOW    EIC_CONFIG_SENSE0_LOW_Val

/* Per-line flags (eic_pin_config flags) */
#define EIC_PIN_IRQ     (1u << 0)   /* interrupt enable (INTENSET) */
#define EIC_PIN_FILTER  (1u << 1)   /* 3-sample filter (FILTEN) */
#define EIC_PIN_EVENT   (1u << 2)   /* event output to EVSYS (EXTINTEO) */

/* Interrupt callback. Called from EIC_Handler (ISR context). */
typedef void (*eic_cb_t)(void);

/* Configures the EXTINT line of a pin. The EXTINT number is looked up from a table.
 *   p     : pin (PA()/PB())
 *   sense : EXTINT_RISE / _FALL / _BOTH / _HIGH / _LOW
 *   flags : EIC_PIN_IRQ | EIC_PIN_FILTER | EIC_PIN_EVENT (or 0)
 *   cb    : called in the interrupt (may be NULL)
 * Returns false (and writes nothing) if
 *   - the pin has no EXTINT line (e.g. PA08), or
 *   - the line is already used by another pin (e.g. PA15 and PB15 = EXTINT15). */
bool eic_pin_config(GpioPin p, uint8_t sense, uint8_t flags, eic_cb_t cb);

 int8_t eic_pin_line(GpioPin p);
