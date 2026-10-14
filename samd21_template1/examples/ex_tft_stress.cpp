#include "project_config.h"

#if EXAMPLE_SELECT == EXAMPLE_TFT_STRESS

#include "examples/examples.h"
#include "examples/support/tft_tests.h"

void example_init(void)
{
}

void example_run(void)
{
	aja_lopullinen_stressitesti_2();
}

#endif /* EXAMPLE_TFT_STRESS */