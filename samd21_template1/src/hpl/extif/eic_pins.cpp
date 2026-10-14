/*
 * eic_pins.cpp
 *
 * Created: 30.10.2026 8.15.05
 *  Author: Kari63
 */ 

#include "eic_pins.h"
#include "eic_init.h"
#include "eic_extint_map.h"
#include "gpio_pins.h"
#include "samd21.h"

/* EIC CONFIG register split derived from the CMSIS macros */
#define EXTINT_SENSE_STRIDE   (EIC_CONFIG_SENSE1_Pos - EIC_CONFIG_SENSE0_Pos)
#define EXTINT_LINES_PER_CFG  (32u / EXTINT_SENSE_STRIDE)

#define EXTINT_CFG_IDX(n)     ((n) / EXTINT_LINES_PER_CFG)
#define EXTINT_CFG_POS(n)     (((n) % EXTINT_LINES_PER_CFG) * EXTINT_SENSE_STRIDE)
#define EXTINT_BIT(n)         (1u << (EIC_INTFLAG_EXTINT_Pos + (n)))

static eic_cb_t s_cb[EIC_EXTINT_NUM];

/* Line owner: 0 = free, otherwise (port * 32 + pin + 1) */
static uint8_t s_owner[EIC_EXTINT_NUM];

bool eic_pin_config(GpioPin p, uint8_t sense, uint8_t flags, eic_cb_t cb)
{
	// 1. Pin -> port and EXTINT line
	const uint8_t port = (p.port == &PORT->Group[PORTB]) ? PORTB : PORTA;
	const int8_t  line = extint_line(port, (uint8_t)p.pin);

	if (line < 0) {
		return false;                      // no EXTINT line (NMI or no such pin)
	}

	const uint32_t n   = (uint32_t)line;
	const uint8_t  id  = (uint8_t)(port * 32u + p.pin + 1u);

	if (s_owner[n] != 0 && s_owner[n] != id) {
		return false;                      // line already used by another pin
	}
	s_owner[n] = id;

	const uint32_t idx = EXTINT_CFG_IDX(n);
	const uint32_t pos = EXTINT_CFG_POS(n);

	uint32_t field = EIC_CONFIG_SENSE0(sense);
	if (flags & EIC_PIN_FILTER) {
		field |= EIC_CONFIG_FILTEN0;
	}

	s_cb[n] = cb;

	// 2. Pinmux: EIC is always function A
	gpio_mux(p, GPIO_MUX_A);

	// 3. CONFIG and EVCTRL are enable-protected -> EIC off during the change
	eic_disable();

	// SENSE + FILTEN: RMW so that the other lines keep their settings
	EIC->CONFIG[idx].reg = (EIC->CONFIG[idx].reg &
	~((EIC_CONFIG_SENSE0_Msk | EIC_CONFIG_FILTEN0) << pos)) |
	(field << pos);

	// Event output to EVSYS
	if (flags & EIC_PIN_EVENT) {
		EIC->EVCTRL.reg |= EIC_EVCTRL_EXTINTEO(EXTINT_BIT(n));
		} else {
		EIC->EVCTRL.reg &= ~EIC_EVCTRL_EXTINTEO(EXTINT_BIT(n));
	}

	// Clear the flag, then enable or disable the interrupt
	EIC->INTFLAG.reg = EIC_INTFLAG_EXTINT(EXTINT_BIT(n));

	if (flags & EIC_PIN_IRQ) {
		EIC->INTENSET.reg = EIC_INTENSET_EXTINT(EXTINT_BIT(n));
		} else {
		EIC->INTENCLR.reg = EIC_INTENCLR_EXTINT(EXTINT_BIT(n));
	}

	eic_enable();
	return true;
}

extern "C" void EIC_Handler(void)
{
	const uint32_t flags = EIC->INTFLAG.reg & EIC_INTFLAG_EXTINT_Msk;

	// Clear all detected flags at once
	EIC->INTFLAG.reg = flags;

	for (uint32_t n = 0; n < EIC_EXTINT_NUM; n++) {
		if ((flags & EXTINT_BIT(n)) && s_cb[n]) {
			s_cb[n]();
		}
	}
}

int8_t eic_pin_line(GpioPin p)
{
	const uint8_t port = (p.port == &PORT->Group[PORTB]) ? PORTB : PORTA;
	return extint_line(port, (uint8_t)p.pin);
}
