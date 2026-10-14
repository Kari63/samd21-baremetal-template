/*
 * random.h
 *
 * Created: 2.9.2026 8.50.11
 *  Author: Kari63
 */ 

#pragma once

static inline uint16_t fast_rand16(void)
{
	static uint16_t lfsr = 0xACE1u;
	lfsr = (lfsr >> 1) ^ (-(lfsr & 1u) & 0xB400u);
	return lfsr;
}