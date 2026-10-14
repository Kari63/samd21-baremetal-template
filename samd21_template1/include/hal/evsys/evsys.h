/*
 * evsys.h
 *
 * Created: 18.9.2026 13.14.48
 *  Author: Kari63
 */ 


#pragma once

#include <stdint.h>

void evsys_route_eic_to_tc7(uint8_t extint_line);

void evsys_init(void);
void evsys_sw_trigger(void);
void evsys_route_sw_trig_to_tc6(void);
void evsys_route_tc4_to_dmac0_and_tc5(void);
void evsys_route_tc3_to_adc(void);
void evsys_route_eic_to_tc7(void);

void evsys_channel_configure(uint8_t channel, uint16_t generator_id, uint32_t path);
void evsys_user_configure(uint16_t user_id, uint8_t channel);







void evsys_route_init(void);



