#include "project_config.h"

#if EXAMPLE_SELECT == EXAMPLE_QSPI_FLASH_TEST

#include "examples/support/portattu_flash_testi.h"

static bool test_started;

void example_init(void)
{
	test_started = false;
}

void example_run(void)
{
	if (test_started) {
		return;
	}

	test_started = true;
	portattu_flash_testi();
}

#endif /* EXAMPLE_QSPI_FLASH_TEST */