/*___________________________________________________________________________________________________
Title:
	TFT.h v2.0 - Ported for SAMD21 J18A (EXT3)

	Based on Adafruit-GFX-Library (graphics primitives, GFXfont/GFXglyph
	font format) and Adafruit-ST7735-Library (ST7735 init sequence):
	  Adafruit-GFX:    Copyright (c) 2012 Adafruit Industries, BSD license
	                   (LICENSES/BSD-Adafruit-GFX.txt)
	  Adafruit-ST7735: Written by Limor Fried/Ladyada for Adafruit Industries,
	                   MIT license (LICENSES/MIT-Adafruit-ST7735.txt)
	SAMD21 port and modifications: Copyright 2026 Kari63, Apache-2.0.
_____________________________________________________________________________________________________*/

#ifndef DIS_7735_H
#define DIS_7735_H

#include <stdint.h>
#include "samd21.h"



#define WIDTH  _width
#define HEIGHT _height
extern int16_t _xstart;
extern int16_t _ystart;






/*************************************************************
	USER DEFINED SETTINGS - SAMD21 EXT3
**************************************************************/
// 0 = plain hardware SPI (MISO not used for D/C)
#define MISO_FOR_DC						0

// --- SAMD21 PIN ASSIGNMENT (EXT3 HEADER) ---
// Port groups: 0 = port A, 1 = port B
#define TFT_DC_GROUP                   1   
#define TFT_DC_PIN                     15  

#define TFT_CS_GROUP                    0   
#define TFT_CS_PIN                      17

#define TFT_RESET_GROUP                 0   
#define TFT_RESET_PIN                   16

// Display width and height in pixels
#define DISPLAY_WIDTH				160
#define DISPLAY_HEIGHT				128

// Some display needs this offset during initialization
#define X_OFFSET						0 // default: 2
#define Y_OFFSET						0 // default: 1

// RGB color filter panel - RGB or BGR (MADCTL)
#define ST7735_RGB_FILTER				0x00

// 16-bit ('565') color settings
#define _BLACK							0x0000
#define _WHITE 							0xFFFF
#define _RED 							0xF800
#define _GREEN 							0x07E0
#define _BLUE 							0x001F
#define _CYAN 							0x07FF
#define _MAGENTA 						0xF81F
#define _YELLOW 						0xFFE0
#define _ORANGE 						0xFC00
#define _GREY							0x8410
#define _LIGHT_GREY						0xD69A
#define _DIM_GREY						0x6B4D

/*************************************************************
	END OF USER DEFINED SETTINGS
**************************************************************/

/*************************************************************
	SYSTEM DEFINES
**************************************************************/
#define TEXT_ALIGN_LEFT			0
#define TEXT_ALIGN_RIGHT		1
#define TEXT_ALIGN_CENTER		2

// These control the screen rotation and RGB filter (RGB or BGR)
#define ST7735_MADCTL_MV		0x20
#define ST7735_MADCTL_MX		0x40
#define ST7735_MADCTL_MY		0x80
#define ST7735_MADCTL_REG		(ST7735_RGB_FILTER | ST7735_MADCTL_MV | ST7735_MADCTL_MY)  //ST7735_MADCTL_MX

#define ROTATE_DEGREE_0			0	// no rotation
#define ROTATE_DEGREE_90		1	// rotate 90 degree CCW
#define ROTATE_DEGREE_180		2	// rotate 180 degree CCW
#define ROTATE_DEGREE_270		3	// rotate 270 degree CCW

#define ST_CMD_DELAY 			0x80 // special signifier for command lists

// TFT HEX Commands
#define ST7735_NOP 				0x00
#define ST7735_SWRESET 			0x01
#define ST7735_RDDID 			0x04
#define ST7735_RDDST 			0x09
#define ST7735_RDDMADCTL		0x0B
#define ST7735_RDDCOLMOD		0x0C
#define ST7735_RDID1 			0xDA
#define ST7735_RDID2 			0xDB
#define ST7735_RDID3 			0xDC
#define ST7735_RDID4 			0xDD
#define ST7735_SLPIN  			0x10
#define ST7735_SLPOUT  			0x11
#define ST7735_PTLON  			0x12
#define ST7735_NORON  			0x13
#define ST7735_INVOFF 			0x20
#define ST7735_INVON 			0x21
#define ST7735_DISOFF 			0x28
#define ST7735_DISPON 			0x29
#define ST7735_CASET 			0x2A
#define ST7735_RASET 			0x2B
#define ST7735_RAMWR 			0x2C
#define ST7735_RAMRD 			0x2E
#define ST7735_COLMOD 			0x3A
#define ST7735_MADCTL 			0x36
#define ST7735_FRMCTR1 			0xB1
#define ST7735_FRMCTR2 			0xB2
#define ST7735_FRMCTR3 			0xB3
#define ST7735_INVCTR 			0xB4
#define ST7735_DISSET5 			0xB6
#define ST7735_PWCTR1 			0xC0
#define ST7735_PWCTR2 			0xC1
#define ST7735_PWCTR3 			0xC2
#define ST7735_PWCTR4 			0xC3
#define ST7735_PWCTR5 			0xC4
#define ST7735_VMCTR1 			0xC5
#define ST7735_PWCTR6 			0xFC
#define ST7735_GMCTRP1 			0xE0
#define ST7735_GMCTRN1 			0xE1
#define ST7735_TEOFF 			0x34
#define ST7735_TEON 			0x35
#define ST7735_IDMOFF 			0x38
#define ST7735_IDMON 			0x39

// Used as a parameter for TFT_drawCircleHelper()
#define _TOP_LEFT_CORNER		0x1
#define _TOP_RIGHT_CORNER		0x2
#define _BOTTOM_RIGHT_CORNER	0x4
#define _BOTTOM_LEFT_CORNER		0x8
#define _ALL_CORNERS			0xF

/*************************************************************
	MACRO FUNCTIONS - SAMD21 REKISTERIT
**************************************************************/
#define SPI_DC_HIGH_DATA()		PORT->Group[TFT_DC_GROUP].OUTSET.reg = (1u << TFT_DC_PIN)
#define SPI_DC_LOW_CMD()		PORT->Group[TFT_DC_GROUP].OUTCLR.reg = (1u << TFT_DC_PIN)

#define SPI_TFT_CS_HIGH()		PORT->Group[TFT_CS_GROUP].OUTSET.reg = (1u << TFT_CS_PIN)
#define SPI_TFT_CS_LOW()		PORT->Group[TFT_CS_GROUP].OUTCLR.reg = (1u << TFT_CS_PIN)

#ifndef min
	#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef _swap_int16_t
	#define _swap_int16_t(a, b){                \
		int16_t t = a;                          \
		a = b;                                  \
		b = t;                                  \
	}
#endif

/*************************************************************
	STRUCTURES (AVR-specific types removed)
**************************************************************/
typedef struct {
	uint16_t bitmapOffset; 
	uint8_t width;         
	uint8_t height;        
	uint8_t xAdvance;      
	int8_t xOffset;        
	int8_t yOffset;        
} GFXglyph;

typedef struct {
	const uint8_t *bitmap;  
	const GFXglyph *glyph;  
	uint16_t first;   
	uint16_t last;    
	uint8_t yAdvance; 
} GFXfont;

extern const GFXfont *gfxFont;
extern int16_t _width;		      
extern int16_t _height;
extern int16_t _xstart;
extern int16_t _ystart;

extern uint8_t cursorLineStartX;
		      

/*************************************************************
	FUNCTION PROTOTYPES
**************************************************************/
void TFT_fillScreen(uint16_t color);	//  OK
void TFT_displayInit(void);				//OK
uint16_t TFT_getStringOffset(void);
uint16_t TFT_drawString(const char *c); 
uint8_t TFT_drawChar(int16_t x, int16_t y, unsigned char c);
void TFT_setFont(const GFXfont *f);		//OK
void TFT_setFontBold(bool bold);
void TFT_setFontSize(uint8_t size_x, uint8_t size_y); //ok
void TFT_setFontColor(uint16_t color);   //OK
void TFT_setBackgroundColor(uint16_t color);  // OK
void TFT_textAlign(uint8_t align, uint8_t nrOfPixels);
void TFT_setTextBoundaries(uint8_t left, uint8_t right, uint8_t top, uint8_t bottom);
void TFT_setLineThickness(uint8_t thickness);
void TFT_drawMonochromeBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color, uint16_t bg, uint8_t tr);
void TFT_drawGrayscaleBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h);
void TFT_drawRGBBitmap(int16_t x, int16_t y, const uint16_t bitmap[], int16_t w, int16_t h);
void TFT_drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color); //OK
void TFT_drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);
void TFT_drawCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, uint16_t color);
void TFT_drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);  //OK
void TFT_drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);  //OK
void TFT_drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
void TFT_fillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t corners, int16_t delta, uint16_t color);
void TFT_fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color); //OK
void TFT_fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);
void TFT_displayInit(void);  //OK
void TFT_fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void TFT_fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);  //OK
void TFT_setAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
void TFT_setCursor(uint8_t x, uint8_t y);
void TFT_setCursorX(uint8_t x);
void TFT_setCursorY(uint8_t y);
void TFT_setRotation(uint8_t x); //OK
void TFT_writePixel(int16_t x, int16_t y, uint16_t color);
void TFT_drawVLine(int16_t x, int16_t y, int16_t h, uint16_t color);//OK
void TFT_drawHLine(int16_t x, int16_t y, int16_t w, uint16_t color);//OK
void TFT_gotoNextLine(uint8_t x);
uint16_t rgbConvert24to16(uint32_t rgb24);
uint8_t TFT_getDisplayWidth(void);  //OK
uint8_t TFT_getDisplayHeight(void);//OK
uint8_t TFT_getRowCount(void);//OK
uint8_t TFT_getColumnCount(void);
uint8_t TFT_getCursorX(void);  //OK
uint8_t TFT_getCursorY(void); //OK 
uint8_t TFT_getLineHeight(void);//OK
uint8_t TFT_getLineYStart(uint8_t lineNr);
uint8_t TFT_getTextWidth(const char *c);

uint8_t TFT_getFontScaleX(void);
uint8_t TFT_getFontScaleY(void);


// System
void TFT_writeData(uint8_t data);
void TFT_writeCommand(uint8_t cmd);
void SPI_startWrite(void);
void SPI_transmit(uint32_t data, uint8_t data_size);
void SPI_endWrite(void);

void TFT_sendCommand(uint8_t cmd);
void TFT_sendDataBulk(const uint8_t *data, uint8_t count);


#endif // DIS_7735_H
