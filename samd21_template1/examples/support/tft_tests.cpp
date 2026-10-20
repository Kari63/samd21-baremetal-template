/*
 * tft_tests.cpp
 *
 * Created: 23.9.2026 4.53.56
 *  Author: Kari63
 */ 

#include "samd21.h"
#include "project_config.h"
#include <math.h>
#include "tft.h"
#include "dma_tft.h"
#include "examples/support/tft_tests.h"
#include "random.h"
#include "systick.h"
#include "FreeMono9pt7b.h"
#include "FreeSans12pt7b.h"





static uint8_t stressivaihe = 0;
static uint8_t dma_stressivaihe = 0;
static uint16_t stressi_alkio = 0;
static int16_t stressi_x = 0;
static int16_t stressi_y = 0;
static int16_t stressi_w = 0;
static int16_t stressi_h = 0;
static int16_t stressi_x2 = 0;
static int16_t stressi_y2 = 0;
static uint16_t stressi_vari = 0;
static uint8_t tunnelikehys = 0;
static uint8_t tunnelirengas = 0;
static uint8_t tunneliviiva = 0;
static uint32_t stressi_hold_deadline = 0;
extern volatile uint32_t systick_ms_laskuri;





static uint8_t app2_vaihe = 0;
static uint8_t app2_kehys = 0;
static uint8_t app2_rengas = 0;
static uint8_t app2_viiva = 0;

void app2(void)
{
	if (TFT_fillScreen_DMA_busy()) {
		return;
	}

	switch (app2_vaihe) {
	case 0:
		app2_kehys = 0;
		app2_rengas = 0;
		app2_viiva = 0;
		TFT_fillScreen_DMA(_BLACK);
		app2_vaihe = 1;
		break;

	case 1:
		TFT_drawRect(0, 0, 160, 128, _MAGENTA);
		app2_vaihe = 2;
		break;

	case 2: {
		const int16_t center_x = 80 + (int16_t)(15.0f * std::sin(app2_kehys * 0.3f));
		const int16_t center_y = 64 + (int16_t)(10.0f * std::cos(app2_kehys * 0.4f));
		const int16_t radius = 8 + app2_rengas * 6;
		const int16_t size_x = radius + (int16_t)(6.0f * sinf((app2_kehys + radius) * 0.2f));
		const int16_t size_y = (size_x * 3) / 4;

		switch (app2_viiva) {
		case 0: TFT_drawLine(center_x, center_y - size_y, center_x + size_x, center_y, _GREEN); break;
		case 1: TFT_drawLine(center_x + size_x, center_y, center_x, center_y + size_y, _GREEN); break;
		case 2: TFT_drawLine(center_x, center_y + size_y, center_x - size_x, center_y, _GREEN); break;
		default: TFT_drawLine(center_x - size_x, center_y, center_x, center_y - size_y, _GREEN); break;
		}

		app2_viiva++;
		if (app2_viiva < 4) {
			break;
		}

		app2_viiva = 0;
		app2_rengas++;
		if (app2_rengas < 8) {
			break;
		}

		app2_rengas = 0;
		app2_kehys++;
		TFT_fillRect_DMA(40, 25, 80, 78, _BLACK);
		break;
	}

	default:
		app2_vaihe = 0;
		break;
	}
}

void aja_lopullinen_stressitesti_2(void) {
	if (TFT_fillScreen_DMA_busy()) {
		return;
	}
	
//	GpioPin TEST = PA(16);
//	gpio_init_output(TEST);

	switch (stressivaihe) {
	case 0:
		TFT_fillScreen_DMA(_BLACK);
		stressi_alkio = 0;
		stressivaihe = 1;
		break;

	case 1:
		if (stressi_alkio >= 500) {
			stressi_alkio = 0;
			stressivaihe = 3;
			return;
		}
		stressi_x = fast_rand16() % 300 - 50;
		stressi_y = fast_rand16() % 200 - 40;
		stressi_w = fast_rand16() % 120 + 10;
		stressi_h = fast_rand16() % 100 + 10;
		stressi_vari = fast_rand16() % 0xFFFF;
		TFT_drawRect(stressi_x, stressi_y, stressi_w, stressi_h, stressi_vari);
		stressivaihe = 2;
		break;

	case 2:
		TFT_drawLine(stressi_x, stressi_y,
			stressi_x + stressi_w, stressi_y + stressi_h, stressi_vari);
		stressi_alkio++;
		stressivaihe = 1;
		break;

	case 3:
		TFT_fillScreen_DMA(_BLACK);
		stressi_alkio = 0;
		stressivaihe = 4;
		break;

	case 4:
		if (stressi_alkio >= 200) {
			stressi_alkio = 0;
			stressivaihe = 6;
			return;
		}
		stressi_x = fast_rand16() % 120 + 20;
		stressi_y = fast_rand16() % 90 + 20;
		stressi_w = fast_rand16() % 30 + 5;
		stressi_vari = fast_rand16() % 0xFFFF;
		TFT_fillCircle(stressi_x, stressi_y, stressi_w, stressi_vari);
		stressivaihe = 5;
		break;

	case 5:
	//	TFT_drawCircle(stressi_x, stressi_y, stressi_w, _WHITE);
		stressi_alkio++;
		stressivaihe = 4;
		break;

	case 6:
		TFT_fillScreen_DMA(_BLACK);
		stressi_alkio = 0;
		stressivaihe = 7;
		break;

	case 7:
		if (stressi_alkio >= 200) {
			stressi_hold_deadline = systick_ms_laskuri + 1000;
			stressivaihe = 9;
			return;
		}
		stressi_x = fast_rand16() % 160;
		stressi_y = fast_rand16() % 128;
		stressi_w = fast_rand16() % 160;
		stressi_h = fast_rand16() % 128;
		stressi_x2 = fast_rand16() % 160;
		stressi_y2 = fast_rand16() % 128;
		stressi_vari = fast_rand16() % 0xFFFF;
		
	
		
		TFT_fillTriangle(stressi_x, stressi_y, stressi_w, stressi_h,
			stressi_x2, stressi_y2, stressi_vari);
			
		
			
		stressivaihe = 8;
		break;

	case 8:
		
		TFT_drawTriangle(stressi_x, stressi_y, stressi_w, stressi_h,
		stressi_x2, stressi_y2, _BLACK);
		stressi_alkio++;
			
		stressivaihe = 7;
		break;

	case 9:
	//	if ((int32_t)(systick_ms_laskuri - stressi_hold_deadline) < 0) {
	//		return;
	//	}
		tunnelikehys = 0;
		tunnelirengas = 0;
		tunneliviiva = 0;
		TFT_fillScreen_DMA(_BLACK);
		stressivaihe = 10;
		break;

	case 10:
		TFT_drawRect(0, 0, 160, 128, _MAGENTA);
		stressivaihe = 11;
		break;

	case 11: {
		int16_t center_x = 80 + (int16_t)(15.0f * std::sin(tunnelikehys * 0.3f));
		int16_t center_y = 64 + (int16_t)(10.0f * std::cos(tunnelikehys * 0.4f));
		int16_t radius = 8 + tunnelirengas * 6;
		int16_t size_x = radius + (int16_t)(6.0f * sinf((tunnelikehys + radius) * 0.2f));
		int16_t size_y = (size_x * 3) / 4;
		uint16_t color = (radius % 12 == 0) ? _CYAN : ((radius % 12 == 6) ? _ORANGE : _GREEN);

		switch (tunneliviiva) {
		case 0: TFT_drawLine(center_x, center_y - size_y, center_x + size_x, center_y, color); break;
		case 1: TFT_drawLine(center_x + size_x, center_y, center_x, center_y + size_y, color); break;
		case 2: TFT_drawLine(center_x, center_y + size_y, center_x - size_x, center_y, color); break;
		default: TFT_drawLine(center_x - size_x, center_y, center_x, center_y - size_y, color); break;
		}

		tunneliviiva++;
		if (tunneliviiva >= 4) {
			tunneliviiva = 0;
			tunnelirengas++;
		}
		if (tunnelirengas >= 8) {
			tunnelirengas = 0;
			tunnelikehys++;
			if (tunnelikehys < 20) {
				stressivaihe = 12;
			} else {
				stressivaihe = 0;
			}
		}
		break;
	}

	case 12:
		TFT_fillRect_DMA(40, 25, 80, 78, _BLACK);
		stressivaihe = 13;
		break;

	case 13:
		if (TFT_fillScreen_DMA_busy()) {
			return;
		}
		stressivaihe = 11;
		break;

	default:
		stressivaihe = 0;
		break;
	}
}


void skooppi_gafi_test(void)
{
	
// === DEMO 2: ANIMATED DIGITAL OSCILLOSCOPE ===
while (TFT_fillScreen_DMA_busy()){}
TFT_fillScreen_DMA(_BLACK);

// 1. Draw the grey oscilloscope grid
// horizontal lines
for (int16_t gh = 20; gh < 120; gh += 20) {
	for (int16_t gx = 5; gx < 155; gx += 4) {
		TFT_writePixel(gx, gh, _GREY); // dotted horizontal line
	}
}
// vertical lines
for (int16_t gv = 20; gv < 160; gv += 20) {
	for (int16_t gy = 15; gy < 115; gy += 4) {
		TFT_writePixel(gv, gy, _GREY); // dotted vertical line
	}
}

// Draw the yellow scope frame and the labels
TFT_drawRect(2, 12, 156, 105, _YELLOW);
TFT_setFontColor(_WHITE);
while (TFT_fillScreen_DMA_busy()){}
TFT_fillScreen_DMA(_BLACK);
TFT_setCursor(10, 2);
TFT_drawString("CH1: 120KM/H RUN");

// 2. Run the waveform animation (50 frames)
for (int offset = 0; offset < 50; offset++) {
	int16_t lastX = 4;
	int16_t lastY = 64;
	
	// Draw the wave from left to right inside the scope window
	for (int16_t x = 4; x < 154; x += 2) {
		// Compute the sine wave on the fly (offset moves the wave forward)
		// the float conversion is safe because x is scaled to a fixed range
		float kulma = (float)(x + offset * 4) * 0.08f;
		int16_t y = 64 + (int16_t)(25.0f * sinf(kulma));
		
		// Draw the cyan wave segment from the previous point to the new one
		if (x > 4) {
			TFT_drawLine(lastX, lastY, x, y, _CYAN);
		}
		
		lastX = x;
		lastY = y;
	   }
   }
}


void aja_lopullinen_stressitesti(void) {
	// One call starts at most one DMA operation.
	// Waiting happens through the state on the following main-loop iterations.
	if (TFT_fillScreen_DMA_busy()) {
		return;
	}

	switch (dma_stressivaihe) {
		case 0:
		TFT_fillScreen_DMA(_BLACK);
		dma_stressivaihe = 1;
		break;
		case 1:
		TFT_fillRect_DMA(10, 10, 60, 40, _YELLOW);
		dma_stressivaihe = 2;
		break;
		case 2:
		TFT_fillRect_DMA(90, 10, 60, 40, _CYAN);
		dma_stressivaihe = 3;
		break;
		case 3:
		TFT_drawVLine_DMA(80, 0, 128, _WHITE);
		dma_stressivaihe = 4;
		break;
		case 4:
		TFT_drawHLine_DMA(0, 64, 160, _WHITE);
		dma_stressivaihe = 0;
		break;
		default:
		dma_stressivaihe = 0;
		break;
	}
}

#if USE_SYSTICK_DELAY
/* Text check: default font size 1 and 2, GFXfont size 1 and 2, text right
   after a DMA fill, all four rotations and an opaque counter rewritten in
   place without fillRect (must not flicker).
   Text is opaque: every case sets its background colour explicitly.
   State machine like the stress test: one step per
   example_run() call, holds timed from the clock (no delay_ms), so LCD_update
   and the rest of the main loop keep running. Returns true when done. */
static const char *const rot_nimi[4] = { "ROT 0", "ROT 1", "ROT 2", "ROT 3" };
static const uint8_t rot_jarjestys[4] = { 0, 2, 3, 1 };   // ends in the init rotation 1

bool aja_tekstitesti(void) {
	static uint8_t  vaihe = 0;
	static uint8_t  k = 0;
	static uint32_t t0 = 0;
	static uint32_t hold_ms = 0;

	if (vaihe != 0 && vaihe != 6 && (uint32_t)(systick_ms_laskuri - t0) < hold_ms) {
		return false;                           // hold: main loop keeps running
	}

	switch (vaihe) {
	case 0:
		TFT_fillScreen(_BLACK);

		TFT_setFont(nullptr);
		TFT_setFontSize(1, 1);
		TFT_setFontColor(_WHITE);
		TFT_setBackgroundColor(_BLUE);
		TFT_setCursor(0, 0);
		TFT_drawString("OLETUS 1 ABC 123");

		TFT_setFontSize(2, 2);
		TFT_setFontColor(_YELLOW);
		TFT_setBackgroundColor(_RED);
		TFT_setCursor(0, 12);
		TFT_drawString("KOKO 2");

		TFT_setFont(&FreeMono9pt7b);
		TFT_setFontSize(1, 1);
		TFT_setFontColor(_GREEN);
		TFT_setBackgroundColor(_BLACK);
		TFT_setCursor(0, 50);                   // GFXfont: y = baseline
		TFT_drawString("Mono 9pt Abc");

		TFT_setFontSize(2, 2);
		TFT_setFontColor(_CYAN);
		TFT_setCursor(0, 90);
		TFT_drawString("Ab2");

		TFT_fillRect_DMA(100, 100, 60, 28, _MAGENTA);
		vaihe = 1;
		return false;

	case 1:                                     // text right after the DMA fill
		if (TFT_fillScreen_DMA_busy()) {
			return false;
		}
		TFT_setFont(nullptr);
		TFT_setFontSize(1, 1);
		TFT_setFontColor(_BLACK);
		TFT_setBackgroundColor(_MAGENTA);
		TFT_setCursor(104, 110);
		TFT_drawString("DMA OK");
		t0 = systick_ms_laskuri; hold_ms = 5000;
		k = 0;
		vaihe = 2;
		return false;

	case 2: {                                   // Rotations: red frame on the edge of the whole _width x _height area, 0/2 portrait, 1/3 landscape
		uint8_t r = rot_jarjestys[k];
		TFT_setRotation(r);
		TFT_fillScreen(_BLACK);
		TFT_drawRect(0, 0, _width, _height, _RED);
		TFT_drawLine(0, 0, 20, 0, _GREEN);      // green = top-left corner
		TFT_drawLine(0, 0, 0, 20, _GREEN);
		TFT_setFontSize(2, 2);
		TFT_setFontColor(_WHITE);
		TFT_setBackgroundColor(_BLACK);
		TFT_setCursor(10, 10);
		TFT_drawString(rot_nimi[r]);
		t0 = systick_ms_laskuri; hold_ms = 2500;
		vaihe = (++k < 4) ? 2 : 3;
		return false;
	}

	case 3:                                     // Opaque counter: no fillRect between the values
		TFT_setFontSize(1, 1);
		TFT_fillScreen(_BLACK);
		TFT_setFont(&FreeSans12pt7b);
		TFT_setFontColor(_WHITE);
		TFT_setBackgroundColor(_BLACK);
		TFT_setCursor(10, 30);
		TFT_drawString("No fillRect");
		TFT_setFontColor(_YELLOW);
		TFT_setBackgroundColor(_BLUE);
		k = 0;
		t0 = systick_ms_laskuri; hold_ms = 0;
		vaihe = 4;
		return false;

	case 4: {                                   // 000..199, 25 ms per value, 3 digits = same width
		char txt[4] = { (char)('0' + k / 100), (char)('0' + (k / 10) % 10), (char)('0' + k % 10), '\0' };
		TFT_setCursor(50, 80);
		TFT_drawString(txt);
		t0 = systick_ms_laskuri; hold_ms = 25;
		if (++k >= 200) {
			hold_ms = 1500;
			vaihe = 5;
		}
		return false;
	}

	case 5:
		TFT_setFont(nullptr);
		TFT_fillScreen(_BLACK);
		vaihe = 6;
		return true;

	default:                                    // 6: done
		return true;
	}
}
#endif /* USE_SYSTICK_DELAY */
