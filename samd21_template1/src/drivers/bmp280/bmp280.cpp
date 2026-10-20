#include "project_config.h"

#if USE_BMP280

/*
 * bmp280.cpp - Bosch BMP280, written to the datasheet (BST-BMP280-DS001)
 * Port of the stm32f446ze_BareMetal driver: i2c_0 transfer API, SysTick ms.
 *
 * Compared with the old AVR library (M. Koehler 2017):
 *   - t_fine and the raw values are signed (int32_t): below 0 degC the old
 *     uint32_t version gave garbage, and pressure with it
 *   - calibration in one 24-byte burst, measurement in one 6-byte burst
 *     (old: ~30 calibration transfers, temperature read again for pressure)
 *   - reset completion polled from status.im_update, no fixed 10 + 100 ms
 *   - integer results, no float in the driver
 */
#include "bmp280.h"
#include "i2c_0.h"
#include "systick.h"

/* Registers   [datasheet 4.2 memory map] */
static constexpr uint8_t REG_CALIB     = 0x88;   /* 0x88..0x9F, 24 bytes */
static constexpr uint8_t REG_CHIP_ID   = 0xD0;
static constexpr uint8_t REG_RESET     = 0xE0;
static constexpr uint8_t REG_STATUS    = 0xF3;
static constexpr uint8_t REG_CTRL_MEAS = 0xF4;
static constexpr uint8_t REG_CONFIG    = 0xF5;
static constexpr uint8_t REG_PRESS_MSB = 0xF7;   /* 0xF7..0xFC: press[3], temp[3] */

static constexpr uint8_t CHIP_ID       = 0x58;
static constexpr uint8_t RESET_CMD     = 0xB6;
static constexpr uint8_t STATUS_IM_UPDATE = 0x01;   /* NVM copy to image registers running */
static constexpr uint8_t MODE_NORMAL   = 0x03;

static constexpr uint32_t RESET_TIMEOUT_MS = 10u;   /* start-up time 2 ms */

/* Compensation parameters   [datasheet 3.11.2, table 17] */
static struct
{
	uint16_t T1;
	int16_t  T2, T3;
	uint16_t P1;
	int16_t  P2, P3, P4, P5, P6, P7, P8, P9;
} cal;

static bool bmp280_ok = false;

static bool read_regs(uint8_t reg, uint8_t *buf, uint8_t n)
{
	return i2c_write_read(BMP280_ADDR, &reg, 1, buf, n);
}

static bool write_reg(uint8_t reg, uint8_t val)
{
	const uint8_t b[2] = { reg, val };
	return i2c_write(BMP280_ADDR, b, 2);
}

static inline uint16_t u16_le(const uint8_t *p) { return (uint16_t)(p[0] | (p[1] << 8)); }
static inline int16_t  s16_le(const uint8_t *p) { return (int16_t)u16_le(p); }

bool bmp280_init(void)
{
	bmp280_ok = false;

	uint8_t id;
	if (!read_regs(REG_CHIP_ID, &id, 1) || id != CHIP_ID) {
		return false;
	}

	/* Soft reset, then wait until the NVM calibration is copied (im_update = 0).
	   During the start-up the sensor may NACK: keep polling until the timeout. */
	if (!write_reg(REG_RESET, RESET_CMD)) {
		return false;
	}
	delay_ms(2);
	const uint32_t t0 = systick_ms_laskuri;
	for (;;) {
		uint8_t st;
		if (read_regs(REG_STATUS, &st, 1) && !(st & STATUS_IM_UPDATE)) {
			break;
		}
		if ((systick_ms_laskuri - t0) > RESET_TIMEOUT_MS) {
			return false;
		}
	}

	/* Calibration: 12 little-endian words in one burst */
	uint8_t c[24];
	if (!read_regs(REG_CALIB, c, sizeof(c))) {
		return false;
	}
	cal.T1 = u16_le(&c[0]);
	cal.T2 = s16_le(&c[2]);
	cal.T3 = s16_le(&c[4]);
	cal.P1 = u16_le(&c[6]);
	cal.P2 = s16_le(&c[8]);
	cal.P3 = s16_le(&c[10]);
	cal.P4 = s16_le(&c[12]);
	cal.P5 = s16_le(&c[14]);
	cal.P6 = s16_le(&c[16]);
	cal.P7 = s16_le(&c[18]);
	cal.P8 = s16_le(&c[20]);
	cal.P9 = s16_le(&c[22]);

	/* config first, while still in sleep mode: writes to config in normal
	   mode may be ignored   [datasheet 4.3.5 config, 4.3.4 ctrl_meas] */
	if (!write_reg(REG_CONFIG, (uint8_t)((BMP280_STANDBY << 5) | (BMP280_FILTER << 2)))) {
		return false;
	}
	if (!write_reg(REG_CTRL_MEAS, (uint8_t)((BMP280_OSRS_T << 5) | (BMP280_OSRS_P << 2) | MODE_NORMAL))) {
		return false;
	}

	bmp280_ok = true;
	return true;
}

/* Temperature, 0.01 degC; t_fine for the pressure   [datasheet 3.11.3] */
static int32_t compensate_t(int32_t adc_T, int32_t *t_fine)
{
	const int32_t var1 = ((((adc_T >> 3) - ((int32_t)cal.T1 << 1))) * ((int32_t)cal.T2)) >> 11;
	const int32_t var2 = (((((adc_T >> 4) - ((int32_t)cal.T1)) * ((adc_T >> 4) - ((int32_t)cal.T1))) >> 12)
	                      * ((int32_t)cal.T3)) >> 14;
	*t_fine = var1 + var2;
	return (*t_fine * 5 + 128) >> 8;
}

/* Pressure, Pa in Q24.8 (64-bit version)   [datasheet 3.11.3] */
static uint32_t compensate_p(int32_t adc_P, int32_t t_fine)
{
	int64_t var1 = ((int64_t)t_fine) - 128000;
	int64_t var2 = var1 * var1 * (int64_t)cal.P6;
	var2 = var2 + ((var1 * (int64_t)cal.P5) << 17);
	var2 = var2 + (((int64_t)cal.P4) << 35);
	var1 = ((var1 * var1 * (int64_t)cal.P3) >> 8) + ((var1 * (int64_t)cal.P2) << 12);
	var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)cal.P1) >> 33;
	if (var1 == 0) {
		return 0;                           /* avoid division by zero */
	}
	int64_t p = 1048576 - adc_P;
	p = (((p << 31) - var2) * 3125) / var1;
	var1 = (((int64_t)cal.P9) * (p >> 13) * (p >> 13)) >> 25;
	var2 = (((int64_t)cal.P8) * p) >> 19;
	p = ((p + var1 + var2) >> 8) + (((int64_t)cal.P7) << 4);
	return (uint32_t)p;
}

bool bmp280_read(BMP280_Data_t *d)
{
	if (!bmp280_ok) {
		return false;
	}

	uint8_t b[6];
	if (!read_regs(REG_PRESS_MSB, b, sizeof(b))) {
		return false;
	}

	/* 20-bit raw values, MSB first   [datasheet 4.3.6, 4.3.7] */
	const int32_t adc_P = (int32_t)(((uint32_t)b[0] << 12) | ((uint32_t)b[1] << 4) | (b[2] >> 4));
	const int32_t adc_T = (int32_t)(((uint32_t)b[3] << 12) | ((uint32_t)b[4] << 4) | (b[5] >> 4));

	int32_t t_fine;
	d->temp_c100 = compensate_t(adc_T, &t_fine);
	d->press_pa  = compensate_p(adc_P, t_fine) >> 8;     /* Q24.8 -> Pa */
	return true;
}

#endif /* USE_BMP280 */
