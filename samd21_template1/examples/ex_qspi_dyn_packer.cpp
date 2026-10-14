#include "project_config.h"

#if EXAMPLE_SELECT == EXAMPLE_QSPI_DYN_PACKER

#include "dyn_flash_formatter_write.h"
#include "formatter.h"

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

	const uint32_t start_address = 0x000000UL;
	// The target sector must already be erased. This example deliberately does
	// not call flash_erase().
	flash_dyn_packer_init(start_address);
	print("Writing dynamic data from 0x%06lx\r\n", start_address, 4);
	backend_print("Hello! This text is stored automatically at the right position.\n");
	backend_print("The encoder makes sure that earlier data is not destroyed.\n");
	flash_dyn_packer_flush();

	flash_dyn_packer_init();
	backend_print("Now writing the bug report to Microchip.\n");
	backend_print("It lists every issue found so far.\n");
	flash_dyn_packer_flush();

	flash_raw_dump(start_address, 128);
	flash_dyn_reader(start_address, 4);
}

#endif /* EXAMPLE_QSPI_DYN_PACKER */