#include "project_config.h"

#if USE_DRV8833

#include "samd21.h"
#include "drv8833.h"

#define MOTOR1_IN1   0
#define MOTOR1_IN2   1

void motor_set(int16_t duty)
{
	if (duty > 1200)  duty = 1200;
	if (duty < -1200) duty = -1200;

	if (duty > 0) {
		TCC0->CC[MOTOR1_IN1].reg = duty;
		TCC0->CC[MOTOR1_IN2].reg = 0;
	}
	else if (duty < 0) {
		TCC0->CC[MOTOR1_IN1].reg = 0;
		TCC0->CC[MOTOR1_IN2].reg = -duty;
	}
	else {
		TCC0->CC[MOTOR1_IN1].reg = 0;
		TCC0->CC[MOTOR1_IN2].reg = 0;
	}
}

void motor_brake(void)
{
	TCC0->CC[MOTOR1_IN1].reg = 1200;
	TCC0->CC[MOTOR1_IN2].reg = 1200;
}

#endif /* USE_DRV8833 */