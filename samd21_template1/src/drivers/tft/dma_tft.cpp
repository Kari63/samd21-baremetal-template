/*
 * dma_tft.cpp
 *
 * Created: 20.9.2026 10.43.27
 *  Author: Kari63
 */ 
/*
 * DMA-accelerated screen/area fill. This file owns the STATE of the fill
 * operation (remaining bytes, color buffer) - dma.cpp only provides a
 * generic "send this data, tell me when done" interface, and tft.cpp
 * provides the bus locking (SPI_startWrite/endWrite) and the address window
 * setup (TFT_setAddrWindow). Neither of them knows anything about this
 * module - the dependency only points this way.
 *
 * NOTE: no SERCOM1 interrupt (simpler). The DMAC ISR only reports that a
 * chunk is done. TFT_fillScreen_DMA_busy() is a main-loop service that
 * starts the next chunk and releases CS after the last TXC.
 */

#include "project_config.h"

#if USE_TFT

#include "dma_tft.h"
#include "tft.h"
#include "dma.h"

#if USE_TFT_DMA

#define FILL_CHUNK_BYTES 2048   // same chunk size as in the original dma_fill_screen

static uint8_t          color_chunk[FILL_CHUNK_BYTES];
volatile uint32_t tft_dma_remaining = 0;
volatile uint8_t tft_dma_busy = 0;
volatile uint8_t tft_dma_waiting_for_txc = 0;
static volatile uint8_t tft_dma_chunk_complete = 0;

static void dma_finish_if_tx_complete(void)
{
	if (tft_dma_waiting_for_txc && SERCOM1->SPI.INTFLAG.bit.TXC) {
		SPI_endWrite();
		tft_dma_waiting_for_txc = 0;
		tft_dma_busy = 0;
	}
}

static void tft_dma_start_next_chunk(void)
{
	if (tft_dma_remaining == 0) {
		tft_dma_waiting_for_txc = 1;
		return;
	}

	uint16_t chunk = (tft_dma_remaining < FILL_CHUNK_BYTES)
		? (uint16_t)tft_dma_remaining
		: (uint16_t)FILL_CHUNK_BYTES;
	tft_dma_remaining -= chunk;

	dmac_start_spi_transfer(color_chunk, chunk);
}

// Called from DMAC_Handler when the TX channel completes. It does not
// change SERCOM or DMAC configuration.
static void fill_chunk_complete_cb(bool ok)
{
	if (!ok) {
		// A DMA error comes from the DMAC ISR. Waiting for TXC here could lock
		// the whole interrupt path if the error stopped the SPI transfer mid-byte.
		SPI_TFT_CS_HIGH();
		tft_dma_waiting_for_txc = 0;
		tft_dma_busy = 0;
		tft_dma_chunk_complete = 0;
		return;
	}

	tft_dma_chunk_complete = 1;
}

// Common core of the fillScreen/fillRect/drawVLine/drawHLine variants.
// Clipping is intentionally identical to TFT_fillRect -
// same early-return and clamp logic, so that all DMA variants
// behave pixel-exactly like their blocking counterparts.
static void dma_fill_rect_start(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
	if (tft_dma_busy) return;

	if (x < 0) { w += x; x = 0; }
	if (y < 0) { h += y; y = 0; }
	if ((x + w) > _width)  w = _width  - x;
	if ((y + h) > _height) h = _height - y;
	if ((w <= 0) || (h <= 0)) return;

	for (uint32_t i = 0; i < FILL_CHUNK_BYTES; i += 2) {
		color_chunk[i]     = color >> 8;
		color_chunk[i + 1] = color & 0xFF;
	}

	tft_dma_remaining = (uint32_t)w * (uint32_t)h * 2;
	tft_dma_busy = 1;
	tft_dma_waiting_for_txc = 0;
	tft_dma_chunk_complete = 0;

	dma_spi_set_complete_callback(fill_chunk_complete_cb);

	SPI_startWrite();
	TFT_setAddrWindow((uint16_t)x, (uint16_t)y, (uint16_t)w, (uint16_t)h);

	tft_dma_start_next_chunk();
}

void TFT_fillScreen_DMA(uint16_t color)
{
	// Rotation aware - honors the current TFT_setRotation() setting,
	// like the blocking TFT_fillScreen().
	dma_fill_rect_start(0, 0, _width, _height, color);
}

void TFT_fillRect_DMA(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
	dma_fill_rect_start(x, y, w, h, color);
}

void TFT_drawVLine_DMA(int16_t x, int16_t y, int16_t h, uint16_t color)
{
	dma_fill_rect_start(x, y, 1, h, color);
}

void TFT_drawHLine_DMA(int16_t x, int16_t y, int16_t w, uint16_t color)
{
	dma_fill_rect_start(x, y, w, 1, color);
}




bool TFT_fillScreen_DMA_busy(void)
{
	if (tft_dma_chunk_complete) {
		tft_dma_chunk_complete = 0;
		tft_dma_start_next_chunk();
	}

	// The last SPI byte completes after the DMA transfer; the decision is made in the main loop.
	dma_finish_if_tx_complete();
	return tft_dma_busy;
}

#endif /* USE_TFT_DMA */

#endif