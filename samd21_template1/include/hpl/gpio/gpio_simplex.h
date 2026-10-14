#pragma once

#include "samd21.h"
#include <stdbool.h>

typedef struct
{
	PortGroup *port;
	uint32_t   pin;
} GpioPin;

/* --- Basic operations (VALUE API) --- */

static inline void gpio_init_output(GpioPin p)
{
	p.port->DIRSET.reg = (1u << p.pin);
}

static inline void gpio_init_input(GpioPin p)
{
	p.port->DIRCLR.reg = (1u << p.pin);
}

static inline void gpio_set(GpioPin p)
{
	p.port->OUTSET.reg = (1u << p.pin);
}

static inline void gpio_clear(GpioPin p)
{
	p.port->OUTCLR.reg = (1u << p.pin);
}

static inline void gpio_toggle(GpioPin p)
{
	p.port->OUTTGL.reg = (1u << p.pin);
}

static inline bool gpio_read(GpioPin p)
{
	return (p.port->IN.reg & (1u << p.pin)) != 0;
}

/* --- Write 0/1 --- */

static inline void gpio_write(GpioPin p, bool value)
{
	if (value)
	p.port->OUTSET.reg = (1u << p.pin);
	else
	p.port->OUTCLR.reg = (1u << p.pin);
}

/* --- Input + Pull-up --- */

static inline void gpio_init_input_pullup(GpioPin p)
{
	p.port->DIRCLR.reg = (1u << p.pin);
	p.port->PINCFG[p.pin].reg = PORT_PINCFG_INEN | PORT_PINCFG_PULLEN;
	p.port->OUTSET.reg = (1u << p.pin);   // pull-up
}

/* --- Input + Pull-down --- */

static inline void gpio_init_input_pulldown(GpioPin p)
{
	p.port->DIRCLR.reg = (1u << p.pin);
	p.port->PINCFG[p.pin].reg = PORT_PINCFG_INEN | PORT_PINCFG_PULLEN;
	p.port->OUTCLR.reg = (1u << p.pin);   // pull-down
}

/* --- Drive strength --- */

static inline void gpio_set_drive_strength(GpioPin p, bool strong)
{
	if (strong)
	p.port->PINCFG[p.pin].reg |= PORT_PINCFG_DRVSTR;
	else
	p.port->PINCFG[p.pin].reg &= ~PORT_PINCFG_DRVSTR;
}

/* --- Peripheral mux (SERCOM, SPI, I2C, etc.) --- */

static inline void gpio_mux(GpioPin p, uint8_t mux)
{
	p.port->PINCFG[p.pin].reg |= PORT_PINCFG_PMUXEN;
	uint8_t index = p.pin >> 1;
	if (p.pin & 1)
	p.port->PMUX[index].bit.PMUXO = mux;
	else
	p.port->PMUX[index].bit.PMUXE = mux;
}
