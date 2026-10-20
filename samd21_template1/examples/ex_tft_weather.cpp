/*
 * ex_tft_weather.cpp - clock, temperature, humidity and pressure on the TFT
 *
 * Layout (landscape 160 x 128):
 *   header         default font, size 1
 *   hh:mm:ss       FreeSans 18pt, centred
 *   dd.mm.20yy     FreeMono 9pt, centred
 *   T   23.2 C     FreeMono 9pt   DHT20 temperature (+-0.5 degC)
 *   RH  46.3 %     FreeMono 9pt   DHT20 humidity
 *   P 1023.0 hPa   FreeMono 9pt   BMP280 pressure (station pressure)
 *
 * Text through the formatter: TFT_setCursor + print(..., FB_TFT), opaque
 * text, no fillRect, no flicker. FreeMono is monospaced: the values are
 * right-aligned with spaces to a fixed width, so 9.5 -> 10.2 still covers
 * the old text exactly.
 *
 * The sensors measure by themselves (DHT20 state machine every 2 s, BMP280
 * normal mode every 1 s); dht20_read() / bmp280_read() only return the
 * latest values, no bus traffic. The screen is refreshed every SENSOR_MS.
 *
 * Port of the stm32f446ze_BareMetal example (SysTick ms, rtc_calendar2).
 *
 * State machine, one step per example_run(): clear (DMA), static layout, then
 * the clock when the second changes and the sensors every SENSOR_MS.
 */
#include "project_config.h"

#if EXAMPLE_SELECT == EXAMPLE_TFT_WEATHER

#include "examples/examples.h"

#include "tft.h"
#include "dma_tft.h"
#include "systick.h"
#include "formatter.h"
#include "rtc_calendar2.h"
#include "dht20.h"
#include "bmp280.h"
#include "FreeSans18pt7b.h"
#include "FreeMono9pt7b.h"

#define SENSOR_MS       1000u
#define FB_TFT          1           /* fb_putc() backend 1 = TFT */

/* GFXfont y = baseline. FreeSans18: 26 rows above the baseline, cell 42 rows.
   FreeMono9: 11 above, cell 18 rows, 11 px per character. */
#define CLOCK_Y         40          /* cell 14..55 */
#define DATE_X          25          /* 10 characters x 11 px = 110 px, centred */
#define DATE_Y          67          /* cell 56..73 */
#define LINE_X          14          /* 12 characters x 11 px = 132 px */
#define TEMP_Y          85          /* cell 74..91 */
#define RH_Y            103         /* cell 92..109 */
#define PRESS_Y         121         /* cell 110..127 */
#define VALUE_W         6           /* "-12.3", "1023.0" right-aligned */

#define COL_BG          _BLACK
#define COL_HEADER_BG   _DIM_GREY
#define COL_CLOCK       _WHITE
#define COL_DATE        _LIGHT_GREY
#define COL_TEMP        _ORANGE
#define COL_RH          _CYAN
#define COL_PRESS       _GREEN

static uint8_t  vaihe = 0;
static uint32_t t_sensor = 0;
static int16_t  clock_x = 0;
static uint8_t  shown_second = 0xFF;    /* 0xFF = draw on the next round */

/* 0.01 units -> one decimal, right-aligned with spaces to width characters:
   2319 -> "  23.2", -505 -> " -5.1", 102304 -> "1023.0". */
static void fmt_dec1(char *out, int32_t v_c100, uint8_t width)
{
	const bool neg = v_c100 < 0;
	uint32_t a = (uint32_t)(neg ? -v_c100 : v_c100);
	a = (a + 5u) / 10u;                     /* 0.1 units, rounded */

	char tmp[12];
	uint8_t n = 0;
	tmp[n++] = (char)('0' + a % 10u);
	tmp[n++] = '.';
	a /= 10u;
	do {
		tmp[n++] = (char)('0' + a % 10u);
		a /= 10u;
	} while (a != 0u);
	if (neg) {
		tmp[n++] = '-';
	}

	uint8_t i = 0;
	while ((uint8_t)(i + n) < width) {
		out[i++] = ' ';
	}
	while (n != 0u) {
		out[i++] = tmp[--n];
	}
	out[i] = '\0';
}

static void draw_clock(void)
{
	const RTC_Time_t t = rtc_read_time();
	if (t.second == shown_second) {
		return;
	}
	shown_second = t.second;

	TFT_setFont(&FreeSans18pt7b);
	TFT_setFontColor(COL_CLOCK);
	TFT_setBackgroundColor(COL_BG);
	TFT_setCursor(clock_x, CLOCK_Y);
	print("%02u:%02u:%02u", (uint32_t)t.hour, (uint32_t)t.minute, (uint32_t)t.second, FB_TFT);

	TFT_setFont(&FreeMono9pt7b);
	TFT_setFontColor(COL_DATE);
	TFT_setCursor(DATE_X, DATE_Y);
	print("%02u.%02u.20%02u", (uint32_t)t.day, (uint32_t)t.month, (uint32_t)t.year, FB_TFT);
}

static void draw_sensors(void)
{
	char v[12];

	TFT_setFont(&FreeMono9pt7b);
	TFT_setBackgroundColor(COL_BG);

	DHT20_Data_t h;
	const bool h_ok = dht20_read(&h);

	TFT_setFontColor(COL_TEMP);
	TFT_setCursor(LINE_X, TEMP_Y);
	if (h_ok) fmt_dec1(v, h.temp_c100, VALUE_W);
	print("T  %s C", h_ok ? v : "  --.-", FB_TFT);

	TFT_setFontColor(COL_RH);
	TFT_setCursor(LINE_X, RH_Y);
	if (h_ok) fmt_dec1(v, (int32_t)h.rh_c100, VALUE_W);
	print("RH %s %s", h_ok ? v : "  --.-", "%", FB_TFT);   /* no %% in the formatter */

	BMP280_Data_t p;
	const bool p_ok = bmp280_read(&p);

	TFT_setFontColor(COL_PRESS);
	TFT_setCursor(LINE_X, PRESS_Y);
	if (p_ok) fmt_dec1(v, (int32_t)p.press_pa, VALUE_W);  /* Pa = 0.01 hPa */
	print("P  %s hPa", p_ok ? v : "  --.-", FB_TFT);
}

static void draw_layout(void)
{
	TFT_fillRect(0, 0, 160, 13, COL_HEADER_BG);
	TFT_setFont(nullptr);
	TFT_setFontSize(1, 1);
	TFT_setFontColor(_WHITE);
	TFT_setBackgroundColor(COL_HEADER_BG);
	TFT_setCursor(4, 3);
	print("RTC  DHT20  BMP280", FB_TFT);

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
		t_sensor = systick_ms_laskuri - SENSOR_MS;
		vaihe = 2;
		break;

	default:
		draw_clock();
		if ((uint32_t)(systick_ms_laskuri - t_sensor) >= SENSOR_MS) {
			t_sensor += SENSOR_MS;
			draw_sensors();
		}
		break;
	}
}

#endif /* EXAMPLE_TFT_WEATHER */
