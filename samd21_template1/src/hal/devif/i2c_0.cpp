

#include "project_config.h"

#if USE_I2C

#include "i2c_0.h"
#include <stdint.h>
#include "samd21.h"

void i2c_start_write(uint8_t addr)
{
	SERCOM0->I2CM.INTFLAG.reg = SERCOM_I2CM_INTFLAG_MB | SERCOM_I2CM_INTFLAG_SB;
	SERCOM0->I2CM.ADDR.reg = (addr << 1) | 0;
	while ((SERCOM0->I2CM.INTFLAG.reg & (SERCOM_I2CM_INTFLAG_MB | SERCOM_I2CM_INTFLAG_SB)) == 0);
}

void i2c_write(uint8_t data)
{
	SERCOM0->I2CM.DATA.reg = data;
	while (SERCOM0->I2CM.INTFLAG.bit.MB == 0);
}

void i2c_start_read(uint8_t addr)
{
	SERCOM0->I2CM.INTFLAG.reg = SERCOM_I2CM_INTFLAG_MB | SERCOM_I2CM_INTFLAG_SB;
	SERCOM0->I2CM.ADDR.reg = (addr << 1) | 1;

	while ((SERCOM0->I2CM.INTFLAG.reg &
	(SERCOM_I2CM_INTFLAG_MB | SERCOM_I2CM_INTFLAG_SB)) == 0);

	if (SERCOM0->I2CM.INTFLAG.bit.MB || SERCOM0->I2CM.STATUS.bit.RXNACK) {
		SERCOM0->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(3);
		while (SERCOM0->I2CM.SYNCBUSY.bit.SYSOP);
	}
}

uint8_t i2c_read_ack(void)
{
	SERCOM0->I2CM.CTRLB.reg &= ~SERCOM_I2CM_CTRLB_ACKACT;
	uint8_t data = SERCOM0->I2CM.DATA.reg;
	return data;
}

uint8_t i2c_read_nak(void)
{
	SERCOM0->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_ACKACT;
	uint8_t data = SERCOM0->I2CM.DATA.reg;

	SERCOM0->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(3);
	while (SERCOM0->I2CM.SYNCBUSY.bit.SYSOP);

	return data;
}



uint8_t i2c_read_nak(uint8_t addr)
{
	/* 1. Clear the flags and send the address + READ bit (1) */
	SERCOM0->I2CM.INTFLAG.reg = SERCOM_I2CM_INTFLAG_MB | SERCOM_I2CM_INTFLAG_SB;
	SERCOM0->I2CM.ADDR.reg = (addr << 1) | 1;

	/* 2. Wait for either flag (MB or SB) */
	while ((SERCOM0->I2CM.INTFLAG.reg & (SERCOM_I2CM_INTFLAG_MB | SERCOM_I2CM_INTFLAG_SB)) == 0);

	/* 3. MB set or NACK in STATUS: the device did not answer its address */
	if (SERCOM0->I2CM.INTFLAG.bit.MB || SERCOM0->I2CM.STATUS.bit.RXNACK) {
		// Release the bus with a STOP so that it does not hang
		SERCOM0->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(3);
		while (SERCOM0->I2CM.SYNCBUSY.bit.SYSOP);
		return 0x00; // return right away
	}

	/* 4. Here the answer was ACK and the data is ready to be read */
	SERCOM0->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_ACKACT; // NACK for the next byte
	uint8_t received_data = SERCOM0->I2CM.DATA.reg;      // read the byte
	
	/* 5. Send the STOP condition */
	SERCOM0->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(3);
	while (SERCOM0->I2CM.SYNCBUSY.bit.SYSOP);

	return received_data;
}

void i2c_stop(void)
{
	SERCOM0->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(3);
	while (SERCOM0->I2CM.SYNCBUSY.bit.SYSOP);
}

#endif

