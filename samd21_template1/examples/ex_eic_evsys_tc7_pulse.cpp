#include "project_config.h"

#if EXAMPLE_SELECT == EXAMPLE_EIC_EVSYS_TC7_PULSE

#include "eic_pins.h"
#include "evsys.h"
#include "gpio_pins.h"
#include "gpio_simplex.h"

static const GpioPin button_pin = PB(00);

void example_init(void)
{
	gpio_init_input_pullup(button_pin);
	evsys_route_eic_to_tc7(eic_pin_line(button_pin));

	if (!eic_pin_config(button_pin, EXTINT_LOW, EIC_PIN_EVENT | EIC_PIN_FILTER, 0)) {
		for (;;) {
		}
	}
}

void example_run(void)
{
}

#endif /* EXAMPLE_EIC_EVSYS_TC7_PULSE */