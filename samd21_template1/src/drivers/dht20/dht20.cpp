/*
 * dht20.cpp
 *
 * Created: 20.8.2026 19.03.05
 *  Author: Kari63
 */ 
#include "project_config.h"

#if USE_DHT20

#include "dht20.h"
#include "i2c_0.h"

static const uint8_t DHT20_ADDR = 0x38;

/* Trigger command: AC 33 00 */
void DHT20_Init(void)
{
	i2c_start_write(DHT20_ADDR);
	i2c_write(0xAC);
	i2c_write(0x33);
	i2c_write(0x00);
	i2c_stop();

	/* short delay */
	for (volatile uint32_t i=0; i<50000; i++) __asm__("nop");
}

uint8_t DHT20_Read(float *temperature, float *humidity)
{
    uint8_t data[7];

    /* Start a measurement */
    i2c_start_write(DHT20_ADDR);
    i2c_write(0xAC);
    i2c_write(0x33);
    i2c_write(0x00);
    i2c_stop();

    for (volatile uint32_t i=0; i<50000; i++) __asm__("nop");

    /* Read 7 bytes */
    i2c_start_read(DHT20_ADDR);

    for (int i = 0; i < 6; i++)
    {
        data[i] = i2c_read_ack();

        /* DHT20 clock stretching / byte-to-byte preparation */
        for (volatile uint32_t d=0; d<3000; d++) __asm__("nop");
    }

    data[6] = i2c_read_nak();   /* last byte NACK + STOP */

    /* Decode the humidity */
    uint32_t hum = ((uint32_t)data[1] << 12) |
                   ((uint32_t)data[2] << 4)  |
                   (data[3] >> 4);

    *humidity = (hum * 100.0f) / 1048576.0f;

    /* Decode the temperature */
    uint32_t temp = ((uint32_t)(data[3] & 0x0F) << 16) |
                    ((uint32_t)data[4] << 8) |
                    data[5];

    *temperature = (temp * 200.0f) / 1048576.0f - 50.0f;

    return 1;
}

#endif /* USE_DHT20 */

