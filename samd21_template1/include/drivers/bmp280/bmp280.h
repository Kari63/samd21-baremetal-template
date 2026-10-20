#pragma once
/*
 * bmp280.h - Bosch BMP280 temperature / pressure, I2C (SERCOM0)
 * Port of the stm32f446ze_BareMetal driver (same API).
 * Seeed Grove "Barometer Sensor (BMP280)" from the Arduino Sensor Kit: 0x77
 * (SDO high). 0x76 when SDO is low.
 *
 * Normal mode: the sensor measures by itself every BMP280_STANDBY, the result
 * registers always hold the latest measurement. bmp280_read() is one 6-byte
 * burst read (~200 us at 400 kHz), no waiting: safe on every main loop round.
 * The burst keeps temperature and pressure from the same measurement
 * (data registers are shadowed during the read).
 *
 * Results as integers (datasheet compensation, 32/64-bit):
 *   temperature  0.01 degC   (2153 = 21.53 degC, negative below zero)
 *   pressure     Pa          (101325 = 1013.25 hPa)
 *
 * Settings for slow logging (Bosch "weather monitoring"): oversampling x1,
 * IIR filter off, standby 1000 ms. One measurement ~6.4 ms per second:
 * no self-heating. Change the BMP280_* defines below for faster or less
 * noisy data.
 *
 * Datasheet: Bosch BST-BMP280-DS001, ch. 3 (modes, compensation 3.11.3),
 * ch. 4 (memory map)
 */

#include <stdint.h>
#include <stdbool.h>

#define BMP280_ADDR         0x77u   /* 7-bit, SDO high */

/* ctrl_meas osrs_t / osrs_p: 1 = x1, 2 = x2, 3 = x4, 4 = x8, 5 = x16 */
#define BMP280_OSRS_T       1u
#define BMP280_OSRS_P       1u
/* config t_sb: 0 = 0.5 ms, 1 = 62.5, 2 = 125, 3 = 250, 4 = 500, 5 = 1000, 6 = 2000, 7 = 4000 ms */
#define BMP280_STANDBY      5u
/* config filter: 0 = off, 1 = 2, 2 = 4, 3 = 8, 4 = 16 */
#define BMP280_FILTER       0u

typedef struct
{
	int32_t  temp_c100;     /* 0.01 degC */
	uint32_t press_pa;      /* Pa */
} BMP280_Data_t;

/* Chip ID check (0x58), soft reset, calibration, normal mode. Blocks a few
   ms (reset). false = no ACK or wrong chip ID: bmp280_read() then fails. */
bool bmp280_init(void);

/* One burst read + compensation. false = not initialised or I2C error. */
bool bmp280_read(BMP280_Data_t *d);
