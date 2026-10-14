#include "project_config.h"

#if EXAMPLE_SELECT == EXAMPLE_DMA_TFT

#include "dma_tft.h"
#include "examples/examples.h"
#include "tft.h"

static uint8_t color_index;

void example_init(void)
{
	color_index = 0;
	TFT_fillScreen_DMA(_BLACK);
}

void example_run(void)
{
	static const uint16_t colors[] = { _RED, _GREEN, _BLUE, _WHITE };

	if (TFT_fillScreen_DMA_busy()) {
		return;
	}

	TFT_fillScreen_DMA(colors[color_index]);
	color_index = (color_index + 1U) % (sizeof(colors) / sizeof(colors[0]));
}

#endif /* EXAMPLE_DMA_TFT */