/*
 * timer6.h
 *
 * Created: 23.9.2026 18.52.23
 *  Author: Kari63
 */ 

#pragma once
#include <stdint.h>



void tc6_minmax_take(uint16_t &mn, uint16_t &mx);
void tc6_ignore_next_interval(void);
void timer6_onMatch();
void timer6_onOverflow();