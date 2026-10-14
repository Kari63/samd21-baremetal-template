/*
 * eic_init.h
 *
 * Created: 30.10.2026 8.15.05
 *  Author: Kari63
 */ 

#pragma once

/* Initializes the EIC peripheral: APB clock, GCLK0, SWRST, ENABLE and NVIC.
 * Called once before the eic_pin_config() calls. */
void eic_init(void);

/* EIC enable/disable. The CONFIG register is enable-protected, so
 * the SENSE settings must be written with the EIC disabled. */
void eic_enable(void);
void eic_disable(void);


