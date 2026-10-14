/*
 * dma_tft.h
 *
 * DMA-accelerated screen/area fill. Integration layer between dma.cpp
 * (generic SPI DMA) and tft.cpp (blocking drawing driver) -
 * neither of them knows anything about this file.
 */
#pragma once

#include <stdint.h>
#include <stdbool.h>

// Non-blocking, DMA-accelerated full-screen fill with one color.
// Looks the same as TFT_fillScreen() (honors the
// current rotation, _width/_height), but returns immediately -
// the transfer continues in the background through the DMAC interrupt.
void TFT_fillScreen_DMA(uint16_t color);

// Same principle, limited to a rectangle (x,y,w,h). Same clipping
// as the blocking TFT_fillRect.
void TFT_fillRect_DMA(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

// Vertical / horizontal line with DMA. Uses the same fill core
// as fillRect (w=1 or h=1).
void TFT_drawVLine_DMA(int16_t x, int16_t y, int16_t h, uint16_t color);
void TFT_drawHLine_DMA(int16_t x, int16_t y, int16_t w, uint16_t color);



// Returns true while the fill is in progress and at the same time services
// the next TX DMA chunk. Must be called regularly while
// a DMA fill is active.
bool TFT_fillScreen_DMA_busy(void);

// Internal transfer state used by the TFT DMA service.
extern volatile uint8_t tft_dma_busy;
extern volatile uint8_t tft_dma_waiting_for_txc;
extern volatile uint32_t tft_dma_remaining;
