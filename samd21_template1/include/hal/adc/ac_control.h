/*
 * ac_control.h
 *
 * Created: 9.9.2026
 *  Author: Kari63
 */ 

#pragma once
#include <stdint.h>
#include <stdbool.h>


void ac_set_threshold(uint8_t comp_inst, uint8_t value);
bool ac_read_output(uint8_t comp_inst);




void ac_onComp0Match(void);
void ac_onComp1Match(void);
void ac_onWindowMatch(void);
