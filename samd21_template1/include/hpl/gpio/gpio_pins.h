
/*
 *
 * Created: 24.8.2026 22.45.51
 *  Author: Kari63
 */


 #pragma once
 #include "samd21.h"
 #include "gpio_simplex.h"

 #define PORTA 0
 #define PORTB 1

 // C++ compatible macro
 #define PINP(port, pin) \
 GpioPin{ &PORT->Group[port], (pin) }

 #define PA(pin) PINP(PORTA, (pin))
 #define PB(pin) PINP(PORTB, (pin))
 
 #define GPIO_MUX_A  0
 #define GPIO_MUX_B  1
 #define GPIO_MUX_C  2
 #define GPIO_MUX_D  3
 #define GPIO_MUX_E  4
 #define GPIO_MUX_F  5
 #define GPIO_MUX_G  6
 #define GPIO_MUX_H  7

