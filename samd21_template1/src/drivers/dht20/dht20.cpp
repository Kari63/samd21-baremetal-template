#include "project_config.h"

#if USE_DHT20

/*
 * dht20.cpp - DHT20 written to the datasheet, non-blocking state machine
 *
 * Earlier version (this template's first dht20): DHT20_Read() blocked
 * with delay_ms(80) (+ up to 5 x 20 ms) and returned floats. Now the waits
 * are SysTick countdowns, the main loop does one short transfer per step,
 * and the results are integers.
 *
 *   START    status byte: calibrated (0x18)? -> TRIGGER, else retry in 1 s
 *   TRIGGER  write AC 33 00                    -> READ in 80 ms
 *   READ     7 bytes: busy -> READ again in 10 ms (max BUSY_RETRIES),
 *            CRC ok -> store                   -> TRIGGER in PERIOD - 80 ms
 *   any I2C/CRC error                          -> TRIGGER in PERIOD
 */
#include "dht20.h"
#include "i2c_0.h"
#include "systick.h"

static const uint8_t DHT20_ADDR = 0x38;

static constexpr uint32_t POWER_ON_MS   = 100u;    /* datasheet: >= 100 ms after power-on */
static constexpr uint32_t MEASURE_MS    = 80u;     /* datasheet: >= 80 ms after the trigger */
static constexpr uint32_t BUSY_RETRY_MS = 10u;
static constexpr uint8_t  BUSY_RETRIES  = 5u;
static constexpr uint32_t NOT_CAL_MS    = 1000u;

enum { ST_START, ST_TRIGGER, ST_READ };

volatile bool dht20_softirq = false;

static volatile uint32_t s_wait_ms = 0;     /* SysTick countdown; 0 = stopped */
static uint8_t  s_state = ST_START;
static uint8_t  s_busy_tries = 0;
static bool     s_valid = false;
static DHT20_Data_t s_data;

/* Next step after ms. Written only from the main loop when the countdown is
   0 (stopped), so SysTick never decrements it at the same time. */
static void next_in(uint32_t ms)
{
	s_wait_ms = (ms != 0u) ? ms : 1u;
}

void dht20_tick(void)
{
	if (s_wait_ms != 0u && --s_wait_ms == 0u) {
		dht20_softirq = true;
	}
}

void dht20_init(void)
{
	s_state = ST_START;
	s_valid = false;
	const uint32_t now = systick_ms_laskuri;
	next_in((now < POWER_ON_MS) ? (POWER_ON_MS - now) : 1u);
}

/* CRC-8 as in the datasheet: x^8 + x^5 + x^4 + 1, init 0xFF */
static uint8_t dht20_crc8(const uint8_t *data, uint8_t len)
{
	uint8_t crc = 0xFF;
	for (uint8_t i = 0; i < len; i++) {
		crc ^= data[i];
		for (uint8_t b = 0; b < 8; b++) {
			crc = (crc & 0x80) ? (uint8_t)((crc << 1) ^ 0x31) : (uint8_t)(crc << 1);
		}
	}
	return crc;
}

/* 20-bit raw -> 0.01 units without 64-bit math:
   RH = raw / 2^20 * 100 %    -> raw * 10000 / 2^20 = raw * 625 >> 16
   T  = raw / 2^20 * 200 - 50 -> raw * 1250 >> 16 - 5000
   raw < 2^20, so raw * 1250 < 2^31 */
static void store(const uint8_t *d)
{
	const uint32_t hum  = ((uint32_t)d[1] << 12) | ((uint32_t)d[2] << 4) | (d[3] >> 4);
	const uint32_t temp = ((uint32_t)(d[3] & 0x0F) << 16) | ((uint32_t)d[4] << 8) | d[5];

	s_data.rh_c100   = (hum * 625u) >> 16;
	s_data.temp_c100 = (int32_t)((temp * 1250u) >> 16) - 5000;
	s_valid = true;
}

void dht20_softirq_handler(void)
{
	switch (s_state) {
	case ST_START: {
		uint8_t st;
		if (i2c_read(DHT20_ADDR, &st, 1) && (st & 0x18) == 0x18) {
			s_state = ST_TRIGGER;
			next_in(1u);
		} else {
			/* No ACK or not calibrated: the datasheet register init
			   (0x1B/0x1C/0x1E) is not done here, try again later */
			next_in(NOT_CAL_MS);
		}
		break;
	}

	case ST_TRIGGER: {
		static const uint8_t trigger[3] = { 0xAC, 0x33, 0x00 };
		if (i2c_write(DHT20_ADDR, trigger, sizeof(trigger))) {
			s_busy_tries = 0;
			s_state = ST_READ;
			next_in(MEASURE_MS);
		} else {
			next_in(DHT20_PERIOD_MS);
		}
		break;
	}

	case ST_READ: {
		uint8_t d[7];
		if (!i2c_read(DHT20_ADDR, d, sizeof(d))) {
			s_state = ST_TRIGGER;
			next_in(DHT20_PERIOD_MS);
			break;
		}
		if ((d[0] & 0x80) && ++s_busy_tries <= BUSY_RETRIES) {
			next_in(BUSY_RETRY_MS);             /* still measuring */
			break;
		}
		if (!(d[0] & 0x80) && dht20_crc8(d, 6) == d[6]) {
			store(d);
		}
		s_state = ST_TRIGGER;
		next_in(DHT20_PERIOD_MS - MEASURE_MS);
		break;
	}

	default:
		s_state = ST_START;
		next_in(NOT_CAL_MS);
		break;
	}
}

bool dht20_read(DHT20_Data_t *d)
{
	if (!s_valid) {
		return false;
	}
	*d = s_data;
	return true;
}

#endif /* USE_DHT20 */
