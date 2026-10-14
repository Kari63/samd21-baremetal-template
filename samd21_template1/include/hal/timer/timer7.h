/*
 * timer7.h
 *
 * Created: 23.9.2026 18.52.36
 *  Author: Kari63
 */ 

#pragma once
#include <stdint.h>



extern volatile uint32_t  tc7pw ;  // = ((uint32_t)tc7_pulse_ticks * 128u)/ 1000UL;
extern volatile  uint32_t  f_x100; //  = ((uint32_t)781250u / tc7_period_ticks)/100UL;



// Software callbacks
void timer7_onMatch();
void timer7_onOverflow();