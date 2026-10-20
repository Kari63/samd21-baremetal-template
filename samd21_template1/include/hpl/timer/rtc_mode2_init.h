
/*
 * rtc_mode2_init.S
 *
 * Created: 1.9.2026 15.34.02
 *  Author: Kari63
 **/


#pragma once


void rtc_mode2_init(void);

/* Continuous read synchronization of CLOCK (READREQ.RCONT): rtc_read_time()
   reads without waiting. Writing CTRL.ENABLE clears RCONT (measured on the
   board: READREQ 0x4010 -> 0x0010): call this again after every disable /
   enable, otherwise each CLOCK read stalls the CPU ~5 ms (sync at 1.024 kHz,
   interrupts are delayed too). */
void rtc_read_sync_continuous(void);




 
