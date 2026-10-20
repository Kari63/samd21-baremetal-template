

#include "project_config.h"

#if USE_I2C

#include "i2c_0.h"
#include <stdint.h>
#include "samd21.h"
#include "systick.h"
#if USE_I2C_DMA
#include "dma.h"
#endif

void i2c_start_write(uint8_t addr)
{
	i2c_dma_wait();
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
	i2c_dma_wait();
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
	i2c_dma_wait();
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


/* ===================== Transfer API (polled) =====================
   Same API as the STM32 template's fmpi2c1: whole transfers. true = every
   byte ACKed and STOP sent; false = NACK, bus error, arbitration lost or
   timeout. Every wait has a timeout (I2C_TIMEOUT_MS), so a missing or stuck
   device no longer hangs the MCU (the byte functions above wait forever).
   Smart mode (CTRLB.SMEN, sercom0_init) is on: reading DATA sends the
   ACK/NACK in CTRLB.ACKACT and starts the next byte.
   SAM D21 datasheet, SERCOM I2C master: INTFLAG.MB = write done or read
   address NACK, INTFLAG.SB = read byte received, STATUS.RXNACK, BUSERR,
   ARBLOST, BUSSTATE (1 = IDLE, 2 = OWNER). */

static constexpr uint16_t STATUS_BUS_ERRORS = SERCOM_I2CM_STATUS_BUSERR |
                                              SERCOM_I2CM_STATUS_ARBLOST;

static inline void sync_sysop(void)
{
	while (SERCOM0->I2CM.SYNCBUSY.bit.SYSOP);
}

/* Wait for MB or SB; false = timeout */
static bool wait_bus(void)
{
	const uint32_t t0 = systick_ms_laskuri;
	while (!(SERCOM0->I2CM.INTFLAG.reg & (SERCOM_I2CM_INTFLAG_MB | SERCOM_I2CM_INTFLAG_SB))) {
		if ((systick_ms_laskuri - t0) > I2C_TIMEOUT_MS) {
			return false;
		}
	}
	return true;
}

/* End of a failed transfer: STOP if this master still owns the bus, error
   flags clear. After a timeout or a bus error the bus state is forced to
   IDLE (as i2c_dma_end). Returns false for the caller. */
static bool xfer_fail(bool force_idle)
{
	if (SERCOM0->I2CM.STATUS.bit.BUSSTATE == 2) {                 // OWNER
		sync_sysop();
		SERCOM0->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(3);      // STOP
		sync_sysop();
	}
	if (force_idle || (SERCOM0->I2CM.STATUS.reg & STATUS_BUS_ERRORS)) {
		SERCOM0->I2CM.STATUS.reg = STATUS_BUS_ERRORS;             // write 1 = clear
		SERCOM0->I2CM.STATUS.reg = SERCOM_I2CM_STATUS_BUSSTATE(1);
		sync_sysop();
	}
	SERCOM0->I2CM.INTFLAG.reg = SERCOM_I2CM_INTFLAG_MB | SERCOM_I2CM_INTFLAG_SB |
	                            SERCOM_I2CM_INTFLAG_ERROR;
	return false;
}

static inline bool status_bad(void)
{
	return (SERCOM0->I2CM.STATUS.reg & (STATUS_BUS_ERRORS | SERCOM_I2CM_STATUS_RXNACK)) != 0;
}

/* START (or repeated START) + address W + n bytes, no STOP.
   n = 0: address only (probe). 0 = ok, 1 = NACK/error, 2 = timeout. */
static int tx(uint8_t addr, const uint8_t *data, uint8_t n)
{
	sync_sysop();
	SERCOM0->I2CM.ADDR.reg = SERCOM_I2CM_ADDR_ADDR((uint32_t)addr << 1);
	if (!wait_bus()) return 2;
	if (status_bad()) return 1;

	for (uint8_t i = 0; i < n; i++) {
		sync_sysop();
		SERCOM0->I2CM.DATA.reg = data[i];
		if (!wait_bus()) return 2;
		if (status_bad()) return 1;
	}
	return 0;
}

/* START (or repeated START) + address R + n bytes (n >= 1), last byte NACK
   + STOP. Same order as the ASF smart-mode read: ACKACT = 0 for all but the
   last byte; for the last ACKACT = 1, CMD = 3 (NACK + STOP), then DATA.
   0 = ok, 1 = NACK/error, 2 = timeout. */
static int rx(uint8_t addr, uint8_t *data, uint8_t n)
{
	sync_sysop();
	SERCOM0->I2CM.CTRLB.reg &= ~SERCOM_I2CM_CTRLB_ACKACT;
	sync_sysop();
	SERCOM0->I2CM.ADDR.reg = SERCOM_I2CM_ADDR_ADDR(((uint32_t)addr << 1) | 1u);
	if (!wait_bus()) return 2;
	if (SERCOM0->I2CM.INTFLAG.bit.MB || status_bad()) return 1;     // address NACK

	for (uint8_t i = 0; i + 1u < n; i++) {
		sync_sysop();
		data[i] = (uint8_t)SERCOM0->I2CM.DATA.reg;                   // ACK + next byte
		if (!wait_bus()) return 2;
		if (SERCOM0->I2CM.INTFLAG.bit.MB || (SERCOM0->I2CM.STATUS.reg & STATUS_BUS_ERRORS)) return 1;
	}

	sync_sysop();
	SERCOM0->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_ACKACT;
	sync_sysop();
	SERCOM0->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(3);             // NACK + STOP
	sync_sysop();
	data[n - 1u] = (uint8_t)SERCOM0->I2CM.DATA.reg;
	return 0;
}

static void xfer_begin(void)
{
	i2c_dma_wait();
	SERCOM0->I2CM.STATUS.reg  = STATUS_BUS_ERRORS;
	SERCOM0->I2CM.INTFLAG.reg = SERCOM_I2CM_INTFLAG_MB | SERCOM_I2CM_INTFLAG_SB |
	                            SERCOM_I2CM_INTFLAG_ERROR;
}

bool i2c_write(uint8_t addr, const uint8_t *data, uint8_t n)
{
	xfer_begin();
	const int r = tx(addr, data, n);
	if (r != 0) {
		return xfer_fail(r == 2);
	}
	sync_sysop();
	SERCOM0->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(3);             // STOP
	sync_sysop();
	return true;
}

bool i2c_read(uint8_t addr, uint8_t *data, uint8_t n)
{
	if (n == 0u) {
		return false;
	}
	xfer_begin();
	const int r = rx(addr, data, n);
	return (r == 0) ? true : xfer_fail(r == 2);
}

/* Write phase without STOP, then the read phase starts with a repeated START */
bool i2c_write_read(uint8_t addr, const uint8_t *wdata, uint8_t wn,
                    uint8_t *rdata, uint8_t rn)
{
	if (rn == 0u) {
		return false;
	}
	xfer_begin();
	int r = tx(addr, wdata, wn);
	if (r == 0) {
		r = rx(addr, rdata, rn);
	}
	return (r == 0) ? true : xfer_fail(r == 2);
}


#if USE_I2C_DMA
/* --------------------------------------------------------------------
   Write via DMA (DMAC channel I2C_DMA_CHANNEL). SAM D21 datasheet, I2C
   "Master DMA": ADDR is written with ADDR.LENEN = 1 and ADDR.LEN = n; the
   DMA moves n bytes, then the master sends STOP by itself. A NACK before
   n bytes also sends STOP and sets STATUS.LENERR (+ INTFLAG.ERROR).
   Smart mode (CTRLB.SMEN, sercom0_init) is required.
   End: DMAC TCMPL (last byte in DATA) and then BUSSTATE = IDLE (the last
   byte and the STOP are out). Polled from i2c_dma_busy(), no SERCOM IRQ.
-------------------------------------------------------------------- */
static volatile bool s_dma_busy;
static volatile bool s_dma_done;        /* DMAC TCMPL / TERR seen */
static volatile bool s_dma_error;       /* sticky until i2c_dma_take_error() */
static uint32_t      s_dma_t0;
static uint32_t      s_dma_timeout_ms;

static constexpr uint16_t STATUS_ERRORS = SERCOM_I2CM_STATUS_LENERR |
                                          SERCOM_I2CM_STATUS_BUSERR |
                                          SERCOM_I2CM_STATUS_ARBLOST;

static void i2c_dma_complete(bool ok)   /* DMAC_Handler context */
{
	if (!ok) {
		s_dma_error = true;
	}
	s_dma_done = true;
}

/* DMA channel off. After an error or a timeout the bus is put back to a
   known state: STOP if this master still owns it, then BUSSTATE = IDLE. */
static void i2c_dma_end(bool error)
{
	dmac_stop_i2c_transfer();
	if (error) {
		s_dma_error = true;
		SERCOM0->I2CM.STATUS.reg = STATUS_ERRORS;            // write 1 = clear
		if (SERCOM0->I2CM.STATUS.bit.BUSSTATE == 2) {       // OWNER
			SERCOM0->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(3);
			while (SERCOM0->I2CM.SYNCBUSY.bit.SYSOP);
		}
		SERCOM0->I2CM.STATUS.reg = SERCOM_I2CM_STATUS_BUSSTATE(1);
		while (SERCOM0->I2CM.SYNCBUSY.bit.SYSOP);
	}
	SERCOM0->I2CM.INTFLAG.reg = SERCOM_I2CM_INTFLAG_MB | SERCOM_I2CM_INTFLAG_SB |
	                            SERCOM_I2CM_INTFLAG_ERROR;
	s_dma_busy = false;
}

bool i2c_write_dma(uint8_t addr, const uint8_t *data, uint8_t n)
{
	if (n == 0 || i2c_dma_busy()) {
		return false;
	}
	s_dma_done       = false;
	s_dma_t0         = systick_ms_laskuri;
	s_dma_timeout_ms = I2C_TIMEOUT_MS + n / 8u;     /* 125 us/byte: covers 100 kHz (90 us) */
	s_dma_busy       = true;

	dma_i2c_set_complete_callback(i2c_dma_complete);
	SERCOM0->I2CM.STATUS.reg  = STATUS_ERRORS;
	SERCOM0->I2CM.INTFLAG.reg = SERCOM_I2CM_INTFLAG_MB | SERCOM_I2CM_INTFLAG_SB |
	                            SERCOM_I2CM_INTFLAG_ERROR;

	/* DMA first, then the address: the first TX request comes after the address ACK */
	dmac_start_i2c_transfer(data, n);
	SERCOM0->I2CM.ADDR.reg = SERCOM_I2CM_ADDR_ADDR((uint32_t)addr << 1) |
	                         SERCOM_I2CM_ADDR_LENEN |
	                         SERCOM_I2CM_ADDR_LEN(n);
	return true;
}

bool i2c_dma_busy(void)
{
	if (!s_dma_busy) {
		return false;
	}
	if (SERCOM0->I2CM.STATUS.reg & STATUS_ERRORS) {             // NACK (LENERR), bus error, arbitration
		i2c_dma_end(true);
	} else if (s_dma_done && SERCOM0->I2CM.STATUS.bit.BUSSTATE == 1) {    // IDLE: STOP is out
		i2c_dma_end(false);
	} else if ((systick_ms_laskuri - s_dma_t0) > s_dma_timeout_ms) {
		i2c_dma_end(true);
	}
	return s_dma_busy;
}

void i2c_dma_wait(void)
{
	while (i2c_dma_busy()) {
	}
}

bool i2c_dma_take_error(void)
{
	const bool e = s_dma_error;
	s_dma_error = false;
	return e;
}
#endif /* USE_I2C_DMA */

#endif

