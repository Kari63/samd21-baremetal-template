/*
 * pcf8574.cpp - PCF8574 8-bit I2C I/O expander
 *
 * Copyright 2026 Kari63
 * SPDX-License-Identifier: Apache-2.0
 */

#include "project_config.h"

#if USE_LCD

#include "pcf8574.h"
#include "i2c_0.h"


/* Copy of the output register per device. */
static uint8_t pcf8574_out[PCF8574_MAXDEVICES];


/* Only initializes the copy to the power-on state of the chip (all '1').
   Nothing is written to the bus: the user sets the outputs with the first write. */
void pcf8574_init(void)
{
	for (uint8_t i = 0; i < PCF8574_MAXDEVICES; i++) {
		pcf8574_out[i] = 0xFF;
	}
}

/* One byte = one I2C frame. Outputs update at the ACK of the data byte. */
void pcf8574_setoutput(uint8_t deviceid, uint8_t data)
{
	if (deviceid >= PCF8574_MAXDEVICES) {
		return;
	}
	pcf8574_out[deviceid] = data;

	i2c_start_write(PCF8574_ADDRBASE + deviceid);
	i2c_write(data);
	i2c_stop();
}

/* Several states in one I2C frame: outputs update at the ACK of every data
   byte, so each byte is its own pin state and the states are one byte
   transfer time apart (9 bits). */
void pcf8574_setoutputs(uint8_t deviceid, const uint8_t *data, uint8_t count)
{
	if (deviceid >= PCF8574_MAXDEVICES || count == 0) {
		return;
	}
	pcf8574_out[deviceid] = data[count - 1];

	i2c_start_write(PCF8574_ADDRBASE + deviceid);
	for (uint8_t i = 0; i < count; i++) {
		i2c_write(data[i]);
	}
	i2c_stop();
}

/* Any number of states in one I2C frame. USE_I2C_DMA and count <= 255: in
   the background, returns false if the previous write is still running
   (nothing sent, the copy unchanged); data must stay unchanged until
   i2c_dma_busy() is false. Otherwise polled. */
bool pcf8574_write_frame(uint8_t deviceid, const uint8_t *data, uint16_t count)
{
	if (deviceid >= PCF8574_MAXDEVICES || count == 0) {
		return false;
	}
	const uint8_t addr = PCF8574_ADDRBASE + deviceid;
#if USE_I2C_DMA
	if (count <= 255u) {
		if (!i2c_write_dma(addr, data, (uint8_t)count)) {
			return false;
		}
		pcf8574_out[deviceid] = data[count - 1];
		return true;
	}
#endif
	i2c_start_write(addr);
	for (uint16_t i = 0; i < count; i++) {
		i2c_write(data[i]);
	}
	i2c_stop();
	pcf8574_out[deviceid] = data[count - 1];
	return true;
}

/* Returns the last written output state (no bus traffic). */
uint8_t pcf8574_getoutput(uint8_t deviceid)
{
	if (deviceid >= PCF8574_MAXDEVICES) {
		return 0;
	}
	return pcf8574_out[deviceid];
}

/* Reads the pin states. The chip latches the inputs during the ACK bit of
   the read frame. Note: i2c_read_nak() returns 0x00 if the device does not
   acknowledge its address. */
uint8_t pcf8574_getinput(uint8_t deviceid)
{
	if (deviceid >= PCF8574_MAXDEVICES) {
		return 0;
	}
	return i2c_read_nak(PCF8574_ADDRBASE + deviceid);
}

#endif
