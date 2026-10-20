/*
 * tft.cpp - ST7735 TFT driver, ported for SAMD21 J18A (EXT3)
 *
 * Based on Adafruit-GFX-Library (graphics primitives, GFXfont glyph
 * rendering) and Adafruit-ST7735-Library (ST7735 init sequence):
 *   Adafruit-GFX:    Copyright (c) 2012 Adafruit Industries, BSD license
 *                    (LICENSES/BSD-Adafruit-GFX.txt)
 *   Adafruit-ST7735: Written by Limor Fried/Ladyada for Adafruit Industries,
 *                    MIT license (LICENSES/MIT-Adafruit-ST7735.txt)
 * SAMD21 port and modifications: Copyright 2026 Kari63, Apache-2.0.
 */
#include "project_config.h"
#if USE_TFT

#include "tft.h"
#include "spi1.h"
#include "systick.h"  // provides delay_ms
#include <stdlib.h>   // provides abs, malloc etc.
#include <string.h>
#include <algorithm>  // provides std::min in C++
#include <cmath> 


// --- GLOBAL GRAPHICS STATE ---
int16_t _width          = DISPLAY_WIDTH;
int16_t _height         = DISPLAY_HEIGHT;
static int16_t cursor_x        = 0;
static int16_t cursor_y        = 0;
static uint16_t textcolor      = _WHITE;
static uint16_t textbg         = _BLACK;
static uint8_t textsize_x      = 1;
static uint8_t textsize_y      = 1;
static uint8_t rotation        = 1;  //0
static uint8_t lineThickness   = 1;
const GFXfont *gfxFont  = nullptr;

// Text boundaries and alignment
static uint8_t text_align        = TEXT_ALIGN_LEFT;
static uint8_t text_align_pixels = 0;
static uint8_t boundary_left     = 0;
static uint8_t boundary_right    = DISPLAY_WIDTH;
static uint8_t boundary_top      = 0;
static uint8_t boundary_bottom   = DISPLAY_HEIGHT;

// helper for goto next line
uint8_t cursorLineStartX = 0;

// ST7735 full initialization table
static const uint8_t Icmd[] = {
	21,                                 // 21 commands in the list
	0x01, ST_CMD_DELAY, 150,            // SWRESET + 150 ms delay
	0x11, ST_CMD_DELAY, 150,            // SLPOUT + 150 ms delay
	0xB1, 3, 0x00, 0x06, 0x03,          // FRMCTR1
	0xB2, 3, 0x01, 0x2C, 0x2D,          // FRMCTR2
	0xB3, 6, 0x01, 0x2C, 0x2D, 0x01, 0x2C, 0x2D, // FRMCTR3
	0xB4, 1, 0x07,                      // INVCTR
	0xC0, 3, 0xA2, 0x02, 0x84,          // PWCTR1
	0xC1, 1, 0xC5,                      // PWCTR2
	0xC2, 2, 0x0A, 0x00,                // PWCTR3
	0xC3, 2, 0x8A, 0x2A,                // PWCTR4
	0xC4, 2, 0x8A, 0xEE,                // PWCTR5
	0xC5, 1, 0x0E,                      // VMCTR1
	0x20, 0,                            // INVOFF
	0x36, 1, ST7735_MADCTL_REG,         // MADCTL
	0x3A, 1, 0x05,                      // COLMOD (16-bit)
	0x2A, 4, 0x00, 0x00, 0x00, 0x7F,    // CASET (0 - 127)
	0x2B, 4, 0x00, 0x00, 0x00, 0x9F,    // RASET (0 - 159)
	0xE0, 16, 0x02, 0x1c, 0x07, 0x12, 0x37, 0x32, 0x29, 0x2d, 0x29, 0x25, 0x2B, 0x39, 0x00, 0x01, 0x03, 0x10, // GMCTRP1
	0xE1, 16, 0x03, 0x1d, 0x07, 0x06, 0x2E, 0x2C, 0x29, 0x2D, 0x2E, 0x2E, 0x37, 0x3F, 0x00, 0x00, 0x02, 0x10, // GMCTRN1
	0x13, ST_CMD_DELAY, 10,             // NORON
	0x29, ST_CMD_DELAY, 100             // DISPON
};

// Built-in 5x7 default font (read directly from flash)


  static const uint8_t font5x7[] = {
	0x3E, 0x51, 0x49, 0x45, 0x3E, // 0
	0x00, 0x42, 0x7F, 0x40, 0x00, // 1
	0x42, 0x61, 0x51, 0x49, 0x46, // 2
	0x21, 0x41, 0x45, 0x4B, 0x31, // 3
	0x18, 0x14, 0x12, 0x7F, 0x10, // 4
	0x27, 0x45, 0x45, 0x45, 0x39, // 5
	0x3C, 0x4A, 0x49, 0x49, 0x30, // 6
	0x01, 0x71, 0x09, 0x05, 0x03, // 7
	0x36, 0x49, 0x49, 0x49, 0x36, // 8
	0x06, 0x49, 0x49, 0x29, 0x1E, // 9
	0x7E, 0x11, 0x11, 0x11, 0x7E, // A
	0x7F, 0x49, 0x49, 0x49, 0x36, // B
	0x3E, 0x41, 0x41, 0x41, 0x22, // C
	0x7F, 0x41, 0x41, 0x22, 0x1C, // D
	0x7F, 0x49, 0x49, 0x49, 0x41, // E
	0x7F, 0x09, 0x09, 0x09, 0x01, // F
	0x3E, 0x41, 0x49, 0x49, 0x7A, // G
	0x7F, 0x08, 0x08, 0x08, 0x7F, // H
	0x00, 0x41, 0x7F, 0x41, 0x00, // I
	0x20, 0x40, 0x41, 0x3F, 0x01, // J
	0x7F, 0x08, 0x14, 0x22, 0x41, // K
	0x7F, 0x40, 0x40, 0x40, 0x40, // L
	0x7F, 0x02, 0x0C, 0x02, 0x7F, // M
	0x7F, 0x04, 0x08, 0x10, 0x7F, // N
	0x3E, 0x41, 0x41, 0x41, 0x3E, // O
	0x7F, 0x09, 0x09, 0x09, 0x06, // P
	0x3E, 0x41, 0x51, 0x21, 0x5E, // Q
	0x7F, 0x09, 0x19, 0x29, 0x46, // R
	0x46, 0x49, 0x49, 0x49, 0x31, // S
	0x01, 0x01, 0x7F, 0x01, 0x01, // T
	0x3F, 0x40, 0x40, 0x40, 0x3F, // U
	0x1F, 0x20, 0x40, 0x20, 0x1F, // V
	0x3F, 0x40, 0x38, 0x40, 0x3F, // W
	0x63, 0x14, 0x08, 0x14, 0x63, // X
	0x07, 0x08, 0x70, 0x08, 0x07, // Y
	0x61, 0x51, 0x49, 0x45, 0x43, // Z
	/* Punctuation (indexes 36..41), same bitmaps as the classic 5x7 font */
	0x00, 0x60, 0x60, 0x00, 0x00, // .
	0x00, 0x36, 0x36, 0x00, 0x00, // :
	0x08, 0x08, 0x08, 0x08, 0x08, // -
	0x23, 0x13, 0x08, 0x64, 0x62, // %
	0x20, 0x10, 0x08, 0x04, 0x02, // /
	0x08, 0x08, 0x3E, 0x08, 0x08  // +
};
static const char font5x7_punct[] = ".:-%/+";   /* glyphs 36.. in this order */

// --- LOW-LEVEL HARDWARE SYNC FUNCTIONS ---

void SPI_startWrite(void) {
	SPI_TFT_CS_LOW();
}

void SPI_endWrite(void) {
	while (!SERCOM1->SPI.INTFLAG.bit.TXC); 
	SPI_TFT_CS_HIGH();
}

void TFT_writeCommand(uint8_t cmd) {
	SPI_DC_LOW_CMD();
	spi1_transmit(cmd);
	while (!SERCOM1->SPI.INTFLAG.bit.TXC); 
	SPI_DC_HIGH_DATA();
}

void TFT_writeData(uint8_t data) {
	SPI_DC_HIGH_DATA();
	spi1_transmit(data);
}

void SPI_transmit(uint32_t data, uint8_t data_size) {
	switch(data_size) {
		case 8:
			spi1_transmit((uint8_t)data);
			break;
		case 16:
			spi1_transmit((uint8_t)(data >> 8));
			spi1_transmit((uint8_t)data);
			break;
		case 32:
			spi1_transmit((uint8_t)(data >> 24));
			spi1_transmit((uint8_t)(data >> 16));
			spi1_transmit((uint8_t)(data >> 8));
			spi1_transmit((uint8_t)data);
			break;
	}
}

void TFT_sendBulkCommand(uint8_t commandByte, const uint8_t *dataBytes, uint8_t numDataBytes) {
	SPI_startWrite();
	TFT_writeCommand(commandByte);
	for (uint8_t i = 0; i < numDataBytes; i++) {
		spi1_transmit(*dataBytes++);
	}
	SPI_endWrite();
}

void TFT_setAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
	uint16_t x_start = x + X_OFFSET;
	uint16_t y_start = y + Y_OFFSET;
	uint16_t x_end = x_start + w - 1;
	uint16_t y_end = y_start + h - 1;

	TFT_writeCommand(ST7735_CASET);
	spi1_transmit(x_start >> 8);   while (!SERCOM1->SPI.INTFLAG.bit.DRE);
	spi1_transmit(x_start & 0xFF); while (!SERCOM1->SPI.INTFLAG.bit.DRE);
	spi1_transmit(x_end >> 8);     while (!SERCOM1->SPI.INTFLAG.bit.DRE);
	spi1_transmit(x_end & 0xFF);   while (!SERCOM1->SPI.INTFLAG.bit.DRE);

	TFT_writeCommand(ST7735_RASET);
	spi1_transmit(y_start >> 8);   while (!SERCOM1->SPI.INTFLAG.bit.DRE);
	spi1_transmit(y_start & 0xFF); while (!SERCOM1->SPI.INTFLAG.bit.DRE);
	spi1_transmit(y_end >> 8);     while (!SERCOM1->SPI.INTFLAG.bit.DRE);
	spi1_transmit(y_end & 0xFF);   while (!SERCOM1->SPI.INTFLAG.bit.DRE);

	TFT_writeCommand(ST7735_RAMWR);
	while (!SERCOM1->SPI.INTFLAG.bit.TXC); // full sync before the data stream
}



static void TFT_writePixel_internal(int16_t x, int16_t y, uint16_t color) {
	// --- Range check ---
	// Compare as signed 32-bit values (int32_t).
	// Keeps 1x1 pixels and circles working, but stops negative overflows (case 2).
	if ((int32_t)x < 0 || (int32_t)x >= (int32_t)_width || (int32_t)y < 0 || (int32_t)y >= (int32_t)_height) {
		return;
	}
	
	// Set the address window only if the coordinate is valid (0-159 and 0-127)
	TFT_setAddrWindow(x, y, 1, 1);
	
	spi1_transmit(color >> 8);
	spi1_transmit(color & 0xFF);
	while (!SERCOM1->SPI.INTFLAG.bit.TXC);
}



static void TFT_drawVLine_internal(int16_t x, int16_t y, int16_t h, uint16_t color) {
	if ((x < 0) || (x >= _width)) return;
	if (y < 0) { h += y; y = 0; }
	if (y + h > _height) h = _height - y;
	if (h <= 0) return;
	uint8_t ch = color >> 8;
	uint8_t cl = color & 0xFF;
	TFT_setAddrWindow(x, y, 1, h);
	while (h--) {
		spi1_transmit_stream(ch);
		spi1_transmit_stream(cl);
	}
}


static void TFT_drawHLine_internal(int16_t x, int16_t y, int16_t w, uint16_t color) {
	if ((y < 0) || (y >= _height)) return;
	if (x < 0) { w += x; x = 0; }
	if (x + w > _width) w = _width - x;
	if (w <= 0) return;
	uint8_t ch = color >> 8;
	uint8_t cl = color & 0xFF;
	TFT_setAddrWindow(x, y, w, 1);
	while (w--) {
		spi1_transmit_stream(ch);
		spi1_transmit_stream(cl);
	}
}

void TFT_writePixel(int16_t x, int16_t y, uint16_t color) {
	if((x < 0) || (x >= _width) || (y < 0) || (y >= _height)) return;
	SPI_startWrite();
	TFT_writePixel_internal(x, y, color);
	SPI_endWrite();
}


void TFT_fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
	// 1. Fast signed range check up front
	if (x < 0) { w += x; x = 0; }
	if (y < 0) { h += y; y = 0; }
	if ((x + w) > _width)  w = _width  - x;
	if ((y + h) > _height) h = _height - y;
	if ((w <= 0) || (h <= 0)) return;

	uint8_t color_high = color >> 8;
	uint8_t color_low = color & 0xFF;
	uint32_t num_pixels = (uint32_t)w * h;

	// 2. Open the bus and set a single address window for the whole area
	SPI_startWrite();
	TFT_setAddrWindow(x, y, w, h);

	
	while (num_pixels--) {
		spi1_transmit(color_high);
		spi1_transmit(color_low);
	}

	
	SPI_endWrite();
}


void TFT_fillScreen(uint16_t color) {
	TFT_fillRect(0, 0, _width, _height, color);
}

void TFT_drawVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
	SPI_startWrite();
	TFT_drawVLine_internal(x, y, h, color);
	SPI_endWrite();
}

void TFT_drawHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
	SPI_startWrite();
	TFT_drawHLine_internal(x, y, w, color);
	SPI_endWrite();
}


void TFT_drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
	// 1. Bresenham setup calculations
	int16_t steep = abs(y1 - y0) > abs(x1 - x0);
	if (steep) {
		_swap_int16_t(x0, y0);
		_swap_int16_t(x1, y1);
	}
	if (x0 > x1) {
		_swap_int16_t(x0, x1);
		_swap_int16_t(y0, y1);
	}

	int16_t dx = x1 - x0;
	int16_t dy = abs(y1 - y0);
	int16_t err = dx / 2;
	int16_t ystep = (y0 < y1) ? 1 : -1;

	// 2. Open an SPI session for this line
	SPI_startWrite();
	
	// 3. Main drawing loop
	for (; x0 <= x1; x0++) {
		if (steep) {
			TFT_writePixel_internal(y0, x0, color);
			} else {
			TFT_writePixel_internal(x0, y0, color);
		}
		err -= dy;
		if (err < 0) {
			y0 += ystep;
			err += dx;
		}
	}
	
	// 4. Close the SPI session
	SPI_endWrite();
} // end of function


void TFT_drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
	SPI_startWrite();
	TFT_drawHLine_internal(x, y, w, color);
	
	// Uses the _internal variant so that CS is not raised in the middle of the session
	TFT_drawHLine_internal(x, y + h - 1, w, color);
	
	TFT_drawVLine_internal(x, y, h, color);
	TFT_drawVLine_internal(x + w - 1, y, h, color);
	SPI_endWrite();
}

// --- PART 2/3: CIRCLES, ROUNDED SHAPES, TRIANGLES AND BITMAPS ---

void TFT_drawCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, uint16_t color) {
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r; 


	while (x < y) {
		if (f >= 0) {
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;
				// --- Force the side offsets to signed values ---
		if (cornername & 0x4) {
			// Octants 1 & 2 (starting from 6 o'clock)
			int16_t x1 = (int16_t)(x0 + x); int16_t y1 = (int16_t)(y0 + y);
			int16_t x2 = (int16_t)(x0 + y); int16_t y2 = (int16_t)(y0 + x);
			TFT_writePixel_internal(x1, y1, color);
			TFT_writePixel_internal(x2, y2, color);
		}
		if (cornername & 0x2) {
			// Octants 3 & 4 (starting from 12 o'clock)
			int16_t x3 = (int16_t)(x0 + x); int16_t y3 = (int16_t)(y0 - y);
			int16_t x4 = (int16_t)(x0 + y); int16_t y4 = (int16_t)(y0 - x);
			TFT_writePixel_internal(x3, y3, color);
			TFT_writePixel_internal(x4, y4, color);
		}
		if (cornername & 0x1) {
			// Octants 5 & 6 (left side)
			int16_t x5 = (int16_t)(x0 - x); int16_t y5 = (int16_t)(y0 - y);
			int16_t x6 = (int16_t)(x0 - y); int16_t y6 = (int16_t)(y0 - x);
			TFT_writePixel_internal(x5, y5, color);
			TFT_writePixel_internal(x6, y6, color);
		}
		if (cornername & 0x8) {
			// Octants 7 & 8 (right side)
			int16_t x7 = (int16_t)(x0 - x); int16_t y7 = (int16_t)(y0 + y);
			int16_t x8 = (int16_t)(x0 - y); int16_t y8 = (int16_t)(y0 + x);
			TFT_writePixel_internal(x7, y7, color);
			TFT_writePixel_internal(x8, y8, color);
		}

	}
}

 void TFT_drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
/*	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;   */

	SPI_startWrite();
	TFT_writePixel_internal(x0, y0 + r, color);
	TFT_writePixel_internal(x0, y0 - r, color);
	TFT_writePixel_internal(x0 + r, y0, color);
	TFT_writePixel_internal(x0 - r, y0, color);
	TFT_drawCircleHelper(x0, y0, r, 0xF, color);
	SPI_endWrite();
}

void TFT_fillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t corners, int16_t delta, uint16_t color) {
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

	while (x < y) {
		if (f >= 0) {
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		
		

		f += ddF_x;

		if (corners & 0x1) {
			TFT_drawVLine_internal(x0 + x, y0 - y, 2 * y + 1 + delta, color);
			TFT_drawVLine_internal(x0 + y, y0 - x, 2 * x + 1 + delta, color);
		}
		if (corners & 0x2) {
			TFT_drawVLine_internal(x0 - x, y0 - y, 2 * y + 1 + delta, color);
			TFT_drawVLine_internal(x0 - y, y0 - x, 2 * x + 1 + delta, color);
		}
	}
}

void TFT_fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
	SPI_startWrite();
	TFT_drawVLine_internal(x0, y0 - r, 2 * r + 1, color);
	TFT_fillCircleHelper(x0, y0, r, 3, 0, color);
	SPI_endWrite();
}

void TFT_drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
	int16_t max_r = std::min(w, h) / 2;
	if (r > max_r) r = max_r;
	SPI_startWrite();
	TFT_drawHLine_internal(x + r, y, w - 2 * r, color);
	TFT_drawHLine_internal(x + r, y + h - 1, w - 2 * r, color);
	TFT_drawVLine_internal(x, y + r, h - 2 * r, color);
	TFT_drawVLine_internal(x + w - 1, y + r, h - 2 * r, color);
	TFT_drawCircleHelper(x + r, y + r, r, 1, color);
	TFT_drawCircleHelper(x + w - r - 1, y + r, r, 2, color);
	TFT_drawCircleHelper(x + w - r - 1, y + h - r - 1, r, 4, color);
	TFT_drawCircleHelper(x + r, y + h - r - 1, r, 8, color);
	SPI_endWrite();
}


void TFT_fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
	int16_t max_r = std::min(w, h) / 2;
	if (r > max_r) r = max_r;
	
	SPI_startWrite();
	uint8_t color_high = color >> 8;
	uint8_t color_low = color & 0xFF;
	
	// 1. Dimensions of the center part
	int16_t tx = x + r, ty = y;
	int16_t tw = w - 2 * r, th = h;
	if (tx < 0) { tw += tx; tx = 0; }
	if (ty < 0) { th += ty; ty = 0; }
	if (tx + tw > _width)  tw = _width  - tx;
	if (ty + th > _height) th = _height - ty;
	uint32_t num_pixels = ((tw > 0) && (th > 0)) ? (uint32_t)tw * th : 0;
	
	if (num_pixels) TFT_setAddrWindow(tx, ty, tw, th);
	while(num_pixels--) {
		spi1_transmit_stream(color_high);
		spi1_transmit_stream(color_low);
	}

	// 2. Corner centers are computed into separate variables.
	// This avoids integer promotion and offset errors on the 32-bit ARM compiler.
	int16_t cx_oikea = x + w - r - 1; // X center of the right-hand corners
	int16_t cx_vasen = x + r;         // X center of the left-hand corners
	int16_t cy_keski = y + r;         // Y center of the top corners
	
	int16_t delta_korkeus = h - 2 * r - 1; // extra length of the vertical lines between the corners

	// 3. Call the helpers with these coordinates
	TFT_fillCircleHelper(cx_oikea, cy_keski, r, 1, delta_korkeus, color); // right side
	TFT_fillCircleHelper(cx_vasen, cy_keski, r, 2, delta_korkeus, color); // left side
	
	SPI_endWrite();
}


void TFT_drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
	TFT_drawLine(x0, y0, x1, y1, color);
	TFT_drawLine(x1, y1, x2, y2, color);
	TFT_drawLine(x2, y2, x0, y0, color);
}


void TFT_fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
	int16_t a, b, y, last;
	
	// 1. Sort the coordinates by Y (y0 <= y1 <= y2)
	if (y0 > y1) { _swap_int16_t(x0, x1); _swap_int16_t(y0, y1); }
	if (y1 > y2) { _swap_int16_t(x2, x1); _swap_int16_t(y2, y1); }
	if (y0 > y1) { _swap_int16_t(x0, x1); _swap_int16_t(y0, y1); }

	// Keep one SPI session open for the whole operation
	SPI_startWrite();

	if (y0 == y2) { // special case: all points on the same row
		a = b = x0;
		if (x1 < a)      a = x1;
		else if (x1 > b) b = x1;
		if (x2 < a)      a = x2;
		else if (x2 > b) b = x2;
		if (b >= a) TFT_drawHLine_internal(a, y0, b - a + 1, color);
		SPI_endWrite();
		return;
	}

	int16_t dx01 = x1 - x0, dy01 = y1 - y0;
	int16_t dx02 = x2 - x0, dy02 = y2 - y0;
	int16_t dx12 = x2 - x1, dy12 = y2 - y1;
	int32_t sa = 0, sb = 0;

	if (y1 == y2) last = y1;
	else          last = y1 - 1;

	// 2. Draw the upper half of the triangle
	for (y = y0; y <= last; y++) {
		// Guard: avoid division by zero when the height difference is zero
		a = x0 + (dy02 > 0 ? sa / dy02 : 0);
		b = x0 + (dy01 > 0 ? sb / dy01 : 0);
		sa += dx02;
		sb += dx01;
		if (a > b) _swap_int16_t(a, b);
		
		// Make sure the length is positive before touching the hardware
		int16_t leveys = b - a + 1;
		if (leveys > 0) {
			TFT_drawHLine_internal(a, y, leveys, color);
		}
	}

	// 3. Draw the lower half of the triangle
	sa = (int32_t)dx02 * (y1 - y0);
	sb = (int32_t)dx12 * (0);
	for (; y <= y2; y++) {
		// Guard: avoid division by zero in the lower half
		a = x0 + (dy02 > 0 ? sa / dy02 : 0);
		b = x1 + (dy12 > 0 ? sb / dy12 : 0);
		sa += dx02;
		sb += dx12;
		if (a > b) _swap_int16_t(a, b);
		
		int16_t leveys = b - a + 1;
		if (leveys > 0) {
			TFT_drawHLine_internal(a, y, leveys, color);
		}
	}
	
	SPI_endWrite();
}


void TFT_drawMonochromeBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color, uint16_t bg, uint8_t tr) {
	// Bytes per bitmap row (rounded up to a whole byte)
	int16_t byteWidth = (w + 7) / 8;
	uint8_t byte = 0;

	// Keep the SPI bus open for the whole bitmap
	SPI_startWrite();
	
	for (int16_t j = 0; j < h; j++) {
		for (int16_t i = 0; i < w; i++) {
			// MSB-first bit unpacking as in Adafruit-GFX:
			// at the start of a byte (i % 8 == 0) fetch a new byte from memory,
			// otherwise shift the bit pattern left (byte <<= 1).
			if (i & 7) byte <<= 1;
			else       byte = bitmap[j * byteWidth + (i / 8)];
			
			// mask the top bit (0x80)
			if (byte & 0x80) {
				TFT_writePixel_internal(x + i, y + j, color);
				} else if (!tr) {
				// If 'tr' (transparent) is 0, the background pixel is drawn as well
				TFT_writePixel_internal(x + i, y + j, bg);
			}
		}
	}
	
	// Close the SPI session when the whole bitmap is done
	SPI_endWrite();
}


// Bitmap clipping: (x, y, cw, ch) = the part that fits on the screen,
// (bx, by) = its top-left corner inside the bitmap. false = nothing on screen.
static bool clip_bitmap(int16_t *x, int16_t *y, int16_t w, int16_t h,
                        int16_t *bx, int16_t *by, int16_t *cw, int16_t *ch) {
	*bx = 0; *by = 0; *cw = w; *ch = h;
	if (*x < 0) { *cw += *x; *bx = -*x; *x = 0; }
	if (*y < 0) { *ch += *y; *by = -*y; *y = 0; }
	if (*x + *cw > _width)  *cw = _width  - *x;
	if (*y + *ch > _height) *ch = _height - *y;
	return (*cw > 0) && (*ch > 0);
}

void TFT_drawGrayscaleBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h) {
	int16_t bx, by, cw, ch;
	if (!clip_bitmap(&x, &y, w, h, &bx, &by, &cw, &ch)) return;

	SPI_startWrite();
	TFT_setAddrWindow(x, y, cw, ch);
	for (int16_t j = 0; j < ch; j++) {
		const uint8_t *row = &bitmap[(int32_t)(by + j) * w + bx];
		for (int16_t i = 0; i < cw; i++) {
			uint8_t v = row[i];
			uint16_t color = rgbConvert24to16(((uint32_t)v << 16) | ((uint32_t)v << 8) | v);
			spi1_transmit(color >> 8);
			spi1_transmit(color & 0xFF);
		}
	}
	SPI_endWrite();
}

void TFT_drawRGBBitmap(int16_t x, int16_t y, const uint16_t bitmap[], int16_t w, int16_t h) {
	// Clip all edges: without it the ST7735 wraps an image crossing the edge incorrectly
	int16_t bx, by, cw, ch;
	if (!clip_bitmap(&x, &y, w, h, &bx, &by, &cw, &ch)) return;

	// Keep the SPI bus open for the whole image transfer
	SPI_startWrite();
	
	// One address window covering the whole image
	TFT_setAddrWindow(x, y, cw, ch);
	
	// Stream the color bytes straight from the flash table into SERCOM1
	for (int16_t j = 0; j < ch; j++) {
		const uint16_t *row = &bitmap[(int32_t)(by + j) * w + bx];
		for (int16_t i = 0; i < cw; i++) {
			uint16_t color = row[i];
			spi1_transmit_stream(color >> 8);
			spi1_transmit_stream(color & 0xFF);
		}
	}
	
	// Wait until the last bit has left the wire before closing the bus
	while (!SERCOM1->SPI.INTFLAG.bit.TXC);
	SPI_endWrite();
}


// --- PART 3/3: FONT, TEXT, GETTERS AND INITIALIZATION ---

// GFXfont: largest height above the baseline (max -yOffset). The opaque text
// cell starts this many rows above the baseline and is yAdvance rows high.
static uint8_t gfx_ascent = 0;

void TFT_setFont(const GFXfont *f) {
	gfxFont = f;
	gfx_ascent = 0;
	if (f) {
		for (uint16_t i = 0; i <= (uint16_t)(f->last - f->first); i++) {
			const int16_t up = -f->glyph[i].yOffset;
			if (up > gfx_ascent) gfx_ascent = (uint8_t)up;
		}
	}
}
void TFT_setFontBold(bool bold)    { (void)bold; }

void TFT_setFontSize(uint8_t size_x, uint8_t size_y) {
	textsize_x = (size_x > 0) ? size_x : 1;
	textsize_y = (size_y > 0) ? size_y : 1;
}

// Text is always opaque: every pixel of the character cell is written,
// either textcolor or textbg. New text covers the old one, no fillRect first.
void TFT_setFontColor(uint16_t color) { textcolor = color; }
	
void TFT_setBackgroundColor(uint16_t color) { textbg = color; }
	
void TFT_textAlign(uint8_t align, uint8_t nrOfPixels) { text_align = align; text_align_pixels = nrOfPixels; }

void TFT_setTextBoundaries(uint8_t left, uint8_t right, uint8_t top, uint8_t bottom) {
	boundary_left = left; boundary_right = right; boundary_top = top; boundary_bottom = bottom;
}

void TFT_setLineThickness(uint8_t thickness) { lineThickness = (thickness > 0) ? thickness : 1; }
	
 //void TFT_setCursor(uint8_t x, uint8_t y) { cursor_x = x; cursor_y = y;  cursorLineStartX = x;}
	
void TFT_setCursor(uint8_t x, uint8_t y) {
	
	if (y != cursor_y || x < cursor_x) {
	cursorLineStartX = x;
	}
	
	cursor_x = x;
	cursor_y = y;
}	
	
	
	
void TFT_setCursorX(uint8_t x) { cursor_x = x; }
	
void TFT_setCursorY(uint8_t y) { cursor_y = y; }

uint8_t TFT_getTextWidth(const char *c) {
	uint16_t w = 0;
	if (!gfxFont) {
		w = strlen(c) * 6 * textsize_x;
		} else {
		while (*c) {
			uint8_t ch = *c++;
			if ((ch >= gfxFont->first) && (ch <= gfxFont->last)) {
				GFXglyph *glyph = &(((GFXglyph *)gfxFont->glyph)[ch - gfxFont->first]);
				w += glyph->xAdvance * textsize_x;
			}
		}
	}
	return w;
}

uint8_t TFT_getCursorX(void) {
	// Return the internal int16_t cursor_x as uint8_t
	return (uint8_t)cursor_x;
}

uint8_t TFT_getCursorY(void) {
	// Return the internal int16_t cursor_y as uint8_t
	return (uint8_t)cursor_y;
}


void TFT_gotoNextLine(uint8_t x) {
	cursor_x = x;
	cursor_y += gfxFont ? gfxFont->yAdvance * textsize_y : 8 * textsize_y;
	if (cursor_y > boundary_bottom) cursor_y = boundary_top;
}

 



void TFT_setRotation(uint8_t m) {
	rotation = m % 4;
	SPI_startWrite();
	TFT_writeCommand(ST7735_MADCTL);
	// The panel is natively portrait 128 x 160 (CASET 0-127, RASET 0-159), MV turns it landscape.
	// DISPLAY_WIDTH/HEIGHT are the landscape size (160 x 128): 0/2 = portrait, 1/3 = landscape.
	switch (rotation) {
		case 0:
		// transfer (not transmit) keeps the SPI state consistent
		spi1_transmit(ST7735_MADCTL_MX | ST7735_MADCTL_MY | ST7735_RGB_FILTER);
		_width  = DISPLAY_HEIGHT; _height = DISPLAY_WIDTH;
		break;
		case 1:
		// transfer (not transmit) keeps the SPI state consistent
		spi1_transmit(ST7735_MADCTL_MY | ST7735_MADCTL_MV | ST7735_RGB_FILTER);
		_width  = DISPLAY_WIDTH; _height = DISPLAY_HEIGHT;
		break;
		case 2:
		// transfer (not transmit) keeps the SPI state consistent
		spi1_transmit(ST7735_RGB_FILTER);
		_width  = DISPLAY_HEIGHT; _height = DISPLAY_WIDTH;
		break;
		case 3:
		// transfer (not transmit) keeps the SPI state consistent
		spi1_transmit(ST7735_MADCTL_MX | ST7735_MADCTL_MV | ST7735_RGB_FILTER);
		_width  = DISPLAY_WIDTH; _height = DISPLAY_HEIGHT;
		break;
	}
	
	// Wait until the rotation byte has completely left the wire
	while (!SERCOM1->SPI.INTFLAG.bit.TXC);
	
	// Only now may CS be raised
	SPI_endWrite();
}


uint16_t rgbConvert24to16(uint32_t rgb24) {
	uint8_t r = (rgb24 >> 16) & 0xFF, g = (rgb24 >> 8) & 0xFF, b = rgb24 & 0xFF;
	return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

uint8_t TFT_getDisplayWidth(void)   { return _width; }
uint8_t TFT_getDisplayHeight(void)  { return _height; }
uint8_t TFT_getRowCount(void)       { return _height / (gfxFont ? gfxFont->yAdvance : 8); }
uint8_t TFT_getColumnCount(void)    { return _width / 6; }
uint8_t TFT_getLineHeight(void)     { return gfxFont ? gfxFont->yAdvance * textsize_y : 8 * textsize_y; }
uint8_t TFT_getLineYStart(uint8_t lineNr) { return boundary_top + lineNr * TFT_getLineHeight(); }
uint16_t TFT_getStringOffset(void)  { return cursor_x; }

void TFT_displayInit(void) {
	PORT->Group[TFT_DC_GROUP].PINCFG[TFT_DC_PIN].reg = 0;
	PORT->Group[TFT_CS_GROUP].PINCFG[TFT_CS_PIN].reg = 0;
	PORT->Group[TFT_RESET_GROUP].PINCFG[TFT_RESET_PIN].reg = 0;
	// Control pins as outputs
	PORT->Group[TFT_DC_GROUP].DIRSET.reg = (1u << TFT_DC_PIN);
	PORT->Group[TFT_CS_GROUP].DIRSET.reg = (1u << TFT_CS_PIN);
	PORT->Group[TFT_RESET_GROUP].DIRSET.reg = (1u << TFT_RESET_PIN);

	// Initial state high
	SPI_TFT_CS_HIGH();
	SPI_DC_HIGH_DATA();
	PORT->Group[TFT_RESET_GROUP].OUTSET.reg = (1u << TFT_RESET_PIN);

	// Hardware reset
	PORT->Group[TFT_RESET_GROUP].OUTCLR.reg = (1u << TFT_RESET_PIN);
	delay_ms(10);
	PORT->Group[TFT_RESET_GROUP].OUTSET.reg = (1u << TFT_RESET_PIN);
	delay_ms(200);

	// Pull CS low and keep it low for the whole initialization
	SPI_TFT_CS_LOW();

	const uint8_t *addr = Icmd;
	uint8_t numCommands = *addr++;
	
	while (numCommands--) {
		uint8_t cmd = *addr++;
		uint8_t numArgs = *addr++;
		uint8_t ms = numArgs & ST_CMD_DELAY;
		numArgs &= ~(ST_CMD_DELAY);
		
		// Send the command (DC low)
		SPI_DC_LOW_CMD();
		spi1_transmit(cmd);
		while (!SERCOM1->SPI.INTFLAG.bit.TXC); // wait until the command is sent
		
		// Send the arguments (DC high = data)
		SPI_DC_HIGH_DATA();
		for (uint8_t i = 0; i < numArgs; i++) {
			spi1_transmit(*addr++);
			while (!SERCOM1->SPI.INTFLAG.bit.TXC); // wait for each argument
		}
		
		// If the delay bit was set, read the delay length and wait
		if (ms) {
			uint8_t delay_time = *addr++; // the pointer now advances past the delay byte
			delay_ms(delay_time);
		}
	}

	

	// 1. Wait until the last argument has completely left the SERCOM
	while (!SERCOM1->SPI.INTFLAG.bit.TXC);

	// 2. Force D/C low (command mode) before CS rises.
	// This prevents the display from taking a glitch on the CS edge as a color byte.
	SPI_DC_LOW_CMD();
	while (!SERCOM1->SPI.INTFLAG.bit.TXC); // make sure D/C has settled in hardware

	// 3. Release the bus and raise CS
	SPI_TFT_CS_HIGH();
	
	// 4. Leave D/C low (command mode) when idle to protect the display from noise
	SPI_DC_LOW_CMD();
}


/* Opaque text cell: ONE address window for the whole character cell and every
 * pixel streamed as textcolor or textbg (ST7735 auto-increments inside the
 * window). Old text is covered in the same write: no fillRect first, no flicker.
 * One window = 11 bytes, then 2 bytes per pixel (per-pixel windows: 13 bytes
 * per pixel). bit() gets the unscaled font coordinate inside the cell.
 * Clipped to the screen. CS is handled here (SPI_startWrite / SPI_endWrite). */
typedef bool (*cell_bit_fn)(int16_t fx, int16_t fy);

static void draw_text_cell(int16_t x, int16_t y, int16_t w, int16_t h, cell_bit_fn bit) {
	const int16_t x0 = (x < 0) ? 0 : x;
	const int16_t y0 = (y < 0) ? 0 : y;
	const int16_t x1 = (x + w > _width)  ? _width  : (int16_t)(x + w);
	const int16_t y1 = (y + h > _height) ? _height : (int16_t)(y + h);
	if ((x0 >= x1) || (y0 >= y1)) return;

	// Font coordinate as running counters: the M0+ has no divide instruction,
	// so the division runs once per cell and row start only.
	int16_t fy = (y0 - y) / textsize_y, ry = (y0 - y) % textsize_y;
	const int16_t fx0 = (x0 - x) / textsize_x, rx0 = (x0 - x) % textsize_x;

	SPI_startWrite();
	TFT_setAddrWindow(x0, y0, x1 - x0, y1 - y0);
	for (int16_t py = y0; py < y1; py++) {
		int16_t fx = fx0, rx = rx0;
		for (int16_t px = x0; px < x1; px++) {
			const uint16_t color = bit(fx, fy) ? textcolor : textbg;
			spi1_transmit_stream(color >> 8);
			spi1_transmit_stream(color & 0xFF);
			if (++rx == textsize_x) { rx = 0; fx++; }
		}
		if (++ry == textsize_y) { ry = 0; fy++; }
	}
	SPI_endWrite();
}

// Glyph of the current cell (set by TFT_drawChar before draw_text_cell)
static const uint8_t  *cell_5x7;        // 5 column bytes, bit 0 = top row; nullptr = blank
static const GFXglyph *cell_glyph;

// Default font cell 6 x 8: columns 0..4 from the font, column 5 = character spacing
static bool bit_5x7(int16_t fx, int16_t fy) {
	return cell_5x7 && (fx < 5) && ((cell_5x7[fx] >> fy) & 1u);
}

// GFXfont cell xAdvance x yAdvance: x from the cursor, y from baseline - gfx_ascent.
// Glyph bits are one continuous MSB-first stream, width bits per row.
static bool bit_gfx(int16_t fx, int16_t fy) {
	const int16_t gx = fx - cell_glyph->xOffset;
	const int16_t gy = fy - gfx_ascent - cell_glyph->yOffset;
	if ((gx < 0) || (gx >= cell_glyph->width) || (gy < 0) || (gy >= cell_glyph->height)) return false;
	const uint16_t n = (uint16_t)(gy * cell_glyph->width + gx);
	return (gfxFont->bitmap[cell_glyph->bitmapOffset + (n >> 3)] << (n & 7u)) & 0x80u;
}

uint8_t TFT_drawChar(int16_t x, int16_t y, unsigned char c) {

	if (!gfxFont) {
		if ((x >= _width) || (y >= _height) || ((x + 6 * textsize_x - 1) < 0) || ((y + 8 * textsize_y - 1) < 0))
		return 6 * textsize_x;

		uint16_t font_index = 0;

		if (c >= '0' && c <= '9') {
			font_index = c - '0';        // digits start at index 0
			} else if (c >= 'A' && c <= 'Z') {
			font_index = (c - 'A') + 10; // letters start at index 10
			} else if (c >= 'a' && c <= 'z') {
			font_index = (c - 'a') + 10; // lower case is mapped to upper case
			} else {
			font_index = 0;              // unknown characters are printed as zero
			for (uint8_t k = 0; font5x7_punct[k] != '\0'; k++) {
				if (c == (unsigned char)font5x7_punct[k]) {
					font_index = 36 + k; // . : - % / +
					break;
				}
			}
		}

		// Space = blank cell: it erases the old character
		cell_5x7 = (c == ' ') ? nullptr : &font5x7[font_index * 5];
		draw_text_cell(x, y, 6 * textsize_x, 8 * textsize_y, bit_5x7);
		return 6 * textsize_x;
	}
	else {
		if ((c < gfxFont->first) || (c > gfxFont->last)) return 0;
		cell_glyph = &gfxFont->glyph[c - gfxFont->first];

		const int16_t adv = cell_glyph->xAdvance;
		draw_text_cell(x, y - gfx_ascent * textsize_y,
		               adv * textsize_x, gfxFont->yAdvance * textsize_y, bit_gfx);

		// Pixels outside the cell columns (FreeSans '_', some bold glyphs: 1 column)
		// are drawn transparently on top, as before.
		const int16_t xo = cell_glyph->xOffset, w = cell_glyph->width;
		if ((xo < 0) || (xo + w > adv)) {
			const int16_t yo = cell_glyph->yOffset, h = cell_glyph->height;
			for (int16_t gy = 0; gy < h; gy++) {
				for (int16_t gx = 0; gx < w; gx++) {
					const int16_t fx = xo + gx;
					if ((fx >= 0) && (fx < adv)) continue;
					if (!bit_gfx(fx, gfx_ascent + yo + gy)) continue;
					if (textsize_x == 1 && textsize_y == 1) {
						TFT_writePixel(x + fx, y + yo + gy, textcolor);
					} else {
						TFT_fillRect(x + fx * textsize_x, y + (yo + gy) * textsize_y, textsize_x, textsize_y, textcolor);
					}
				}
			}
		}
		return adv * textsize_x;
	}
}


uint16_t TFT_drawString(const char *c) {
	uint16_t len = strlen(c);
	uint16_t total_width = TFT_getTextWidth(c);

	if (text_align == TEXT_ALIGN_RIGHT) {
		cursor_x = text_align_pixels - total_width;
	} else if (text_align == TEXT_ALIGN_CENTER) {
		cursor_x = text_align_pixels - (total_width / 2);
	}

	// Lock the bus at the start ONLY for the default font.
	// The custom font (gfxFont) path handles the bus itself and must not be locked twice.
	if (!gfxFont) {
		SPI_startWrite();
	}

	for (uint16_t i = 0; i < len; i++) {
		if (c[i] == '\n') {
			// When moving to a new line, release the bus for the transition
			if (!gfxFont) SPI_endWrite();
			TFT_gotoNextLine(boundary_left);
			if (!gfxFont) SPI_startWrite();
		} else {
			uint8_t w = TFT_drawChar(cursor_x, cursor_y, c[i]);
			cursor_x += w;
			if (cursor_x > boundary_right) {
				if (!gfxFont) SPI_endWrite();
				TFT_gotoNextLine(boundary_left);
				if (!gfxFont) SPI_startWrite();
			}
		}
	}

	// Release the bus at the end only for the default font
	if (!gfxFont) {
		SPI_endWrite();
	}

	return total_width;
}

#endif /* USE_TFT */







