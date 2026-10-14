/*
 * timer4.h
 *
 * Created: 2.9.2026 16.03.34
 *  Author: Kari63
 */ 


#pragma once
#include <stdint.h>

extern volatile uint16_t next_top4;
extern volatile uint16_t next_duty4;



// Software callbacks
void timer4_onMatch();
void timer4_onOverflow();