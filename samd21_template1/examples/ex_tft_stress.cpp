#include "project_config.h"

#if EXAMPLE_SELECT == EXAMPLE_TFT_STRESS

#include "examples/examples.h"
#include "examples/support/tft_tests.h"

static bool tekstitesti_valmis = false;

void example_init(void)
{
}

void example_run(void)
{
	// Text test first, then the stress test; both one step per call
	if (!tekstitesti_valmis) {
		tekstitesti_valmis = aja_tekstitesti();
		return;
	}
	aja_lopullinen_stressitesti_2();
}

#endif /* EXAMPLE_TFT_STRESS */