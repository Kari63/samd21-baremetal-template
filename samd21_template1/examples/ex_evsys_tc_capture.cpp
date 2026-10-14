#include "project_config.h"

#if EXAMPLE_SELECT == EXAMPLE_EVSYS_TC_CAPTURE

#include "evsys.h"
#include "gpio_pins.h"

const GpioPin test_pin = PA(12);

void example_init(void)
{
	gpio_init_output(test_pin);
	evsys_route_sw_trig_to_tc6();
}

void example_run(void)
{
	evsys_sw_trigger();
	gpio_toggle(test_pin);
}

#endif /* EXAMPLE_EVSYS_TC_CAPTURE */