/*
 * systeminit.h
 */

#pragma once

void systick_init();
void dmac_init();
void eic_init();

void evsys_init();
void adc_init();
void ac_init();

void rtc_mode2_init();
void tcc0_init();
void tc3_init();
void tc4_init();
void tc5_init();
void tc6_init();
void tc7_init();

void flash_init();
void sercom0_init();
void sercom2_init();
void sercom3_init();
void sercom4_init();
void sercom5_init();
void systeminit();
void TFT_displayInit();

void board_gpio_init();