#include "project_config.h"

#if EXAMPLE_SELECT == EXAMPLE_DMA_RAM

#include "dma.h"
#include "examples/support/dma_ram_test.h"
#include "evsys.h"

void example_init(void)
{
	fill_test_pattern();
	dmac_build_channel0_descriptor();
	dmac_arm_channel0_event_trigger();
	evsys_route_tc4_to_dmac0_and_tc5();
}

void example_run(void)
{
}

#endif /* EXAMPLE_DMA_RAM */