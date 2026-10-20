/*
 * dht20.h - DHT20 (AHT20) temperature / humidity, I2C 0x38, non-blocking
 * Grove "Temperature & Humidity Sensor (DHT20)" from the Arduino Sensor Kit.
 * Port of the stm32f446ze_BareMetal driver (replaces the blocking DHT20_Read).
 *
 * Datasheet sequence (Aosong DHT20):
 *   - power-on: >= 100 ms before the first command
 *   - status byte (read 1 byte): bit 3 = calibrated (0x18 mask must be 0x18)
 *   - trigger AC 33 00, wait >= 80 ms, status bit 7 = busy until done
 *   - read 7 bytes: status, 20-bit humidity, 20-bit temperature, CRC-8
 *
 * No delay_ms: a state machine runs one short I2C transfer per step.
 *   SysTick dht20_tick()        counts down the wait, sets dht20_softirq
 *   main loop DHT20_SOFTIRQ_DISPATCH() -> dht20_softirq_handler(): one step
 *   (status check / trigger / read), then the next wait
 * The bus is busy only for the transfers themselves (~0.3 ms per cycle).
 *
 * dht20_read() returns the latest measurement, no bus traffic: any time,
 * also every main loop round. Results as integers:
 *   temperature  0.01 degC   (2153 = 21.53 degC, negative below zero)
 *   humidity     0.01 %RH    (4567 = 45.67 %RH)
 */
#pragma once
#include <stdint.h>
#include <stdbool.h>

#define DHT20_PERIOD_MS     2000u   /* one measurement every 2 s: slow enough, no self-heating */

typedef struct
{
	int32_t  temp_c100;     /* 0.01 degC */
	uint32_t rh_c100;       /* 0.01 %RH */
} DHT20_Data_t;

extern volatile bool dht20_softirq;         /* set by dht20_tick(), cleared by DHT20_SOFTIRQ_DISPATCH */

void dht20_init(void);                      /* starts the state machine, no waiting */
void dht20_tick(void);                      /* from SysTick_Handler, 1 ms */
void dht20_softirq_handler(void);           /* main loop: one step */

/* Latest valid measurement. false = none yet (sensor missing, not
   calibrated, I2C or CRC errors since start). */
bool dht20_read(DHT20_Data_t *d);
