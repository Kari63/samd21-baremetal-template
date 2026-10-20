/*
 * ex_tft_clock_adc.cpp - RTC clock and ADC voltage on the TFT, no flicker
 *
 * Layout (landscape 160 x 128):
 *   header         default font, size 1
 *   hh:mm:ss       FreeSans 18pt, centred
 *   dd.mm.20yy     default font, size 2
 *   x.xxx V        FreeSans 12pt, ADC (adc_last_val)
 *   bar 0..3.3 V   filled part + rest in the background colour
 *
 * Text goes through the formatter: TFT_setCursor + print(..., FB_TFT).
 * No fillRect before the text and no flicker: the TFT text is opaque, every
 * character cell is written once with text and background colour (tft.cpp).
 * The formats have a fixed width, so a new value covers the old one exactly.
 *
 * State machine, one step per example_run(): clear (DMA), static layout, then
 * the clock when the second changes and the ADC every ADC_POLL_MS. No delay_ms.
 */
#include "project_config.h"

#if EXAMPLE_SELECT == EXAMPLE_TFT_CLOCK_ADC

#include "examples/examples.h"

#include "tft.h"
#include "dma_tft.h"
#include "systick.h"
#include "formatter.h"
#include "rtc_calendar2.h"
#include "adc_read.h"
#include "FreeSans12pt7b.h"
#include "FreeSans18pt7b.h"

#define ADC_POLL_MS     200u
#define FB_TFT          1           /* fb_putc() backend 1 = TFT */
#define VREF_MV         3300u

/* Field positions. GFXfont y = baseline, default font y = top. */
#define CLOCK_Y         44          /* FreeSans18: cell 18..59 */
#define DATE_X          20          /* 10 characters x 12 px = 120 px, centred */
#define DATE_Y          62          /* size 2: 16 rows */
#define ADC_X           24
#define ADC_Y           100         /* FreeSans12: cell 83..111 */
#define BAR_X           10
#define BAR_Y           114
#define BAR_W           140
#define BAR_H           10

#define COL_BG          _BLACK
#define COL_HEADER_BG   _DIM_GREY
#define COL_CLOCK       _WHITE
#define COL_DATE        _LIGHT_GREY
#define COL_ADC         _YELLOW
#define COL_BAR         _CYAN
#define COL_FRAME       _GREY

static uint8_t  vaihe = 0;
static uint32_t t_adc = 0;
static int16_t  clock_x = 0;
static uint8_t  shown_second = 0xFF;    /* 0xFF = draw on the next round */

static void draw_clock(void)
{
	const RTC_Time_t t = rtc_read_time();
	if (t.second == shown_second) {
		return;
	}
	shown_second = t.second;

	TFT_setFont(&FreeSans18pt7b);
	TFT_setFontSize(1, 1);
	TFT_setFontColor(COL_CLOCK);
	TFT_setBackgroundColor(COL_BG);
	TFT_setCursor(clock_x, CLOCK_Y);
	print("%02u:%02u:%02u", (uint32_t)t.hour, (uint32_t)t.minute, (uint32_t)t.second, FB_TFT);

	TFT_setFont(nullptr);
	TFT_setFontSize(2, 2);
	TFT_setFontColor(COL_DATE);
	TFT_setCursor(DATE_X, DATE_Y);
	print("%02u.%02u.20%02u", (uint32_t)t.day, (uint32_t)t.month, (uint32_t)t.year, FB_TFT);
}

static void draw_adc(void)
{
	const float v = adc_last_val;

	TFT_setFont(&FreeSans12pt7b);
	TFT_setFontSize(1, 1);
	TFT_setFontColor(COL_ADC);
	TFT_setBackgroundColor(COL_BG);
	TFT_setCursor(ADC_X, ADC_Y);
	print("%3f V", v, FB_TFT);

	/* Bar: filled part + rest in the background colour. The two rectangles do
	   not overlap, so every pixel is written once: no flicker. */
	uint32_t mv = (uint32_t)(v * 1000.0f + 0.5f);
	if (mv > VREF_MV) mv = VREF_MV;
	const int16_t px = (int16_t)(mv * BAR_W / VREF_MV);
	TFT_fillRect(BAR_X,      BAR_Y, px,         BAR_H, COL_BAR);
	TFT_fillRect(BAR_X + px, BAR_Y, BAR_W - px, BAR_H, COL_BG);
}

static void draw_layout(void)
{
	TFT_fillRect(0, 0, 160, 14, COL_HEADER_BG);
	TFT_setFont(nullptr);
	TFT_setFontSize(1, 1);
	TFT_setFontColor(_WHITE);
	TFT_setBackgroundColor(COL_HEADER_BG);
	TFT_setCursor(4, 3);
	print("SAMD21  RTC + ADC", FB_TFT);

	TFT_drawRect(BAR_X - 1, BAR_Y - 1, BAR_W + 2, BAR_H + 2, COL_FRAME);

	/* Clock centred once: fixed format, the width does not change */
	TFT_setFont(&FreeSans18pt7b);
	clock_x = (160 - TFT_getTextWidth("00:00:00")) / 2;

	shown_second = 0xFF;
}

void example_init(void)
{
}

void example_run(void)
{
	switch (vaihe) {
	case 0:
#if USE_TFT_DMA
		TFT_fillScreen_DMA(COL_BG);
#else
		TFT_fillScreen(COL_BG);
#endif
		vaihe = 1;
		break;

	case 1:
#if USE_TFT_DMA
		if (TFT_fillScreen_DMA_busy()) {
			return;                         /* DMA owns SERCOM1 and CS */
		}
#endif
		draw_layout();
		t_adc = systick_ms_laskuri - ADC_POLL_MS;
		vaihe = 2;
		break;

	default:
		draw_clock();
		if ((uint32_t)(systick_ms_laskuri - t_adc) >= ADC_POLL_MS) {
			t_adc += ADC_POLL_MS;
			draw_adc();
		}
		break;
	}
}

#endif /* EXAMPLE_TFT_CLOCK_ADC */
