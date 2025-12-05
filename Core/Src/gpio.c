#include "gpio.h"

void GPIO_SetMode(GPIO_TypeDef* port, uint8_t pin, uint8_t mode) {
	port->MODER &= ~(0b11 << (pin*2));
	port->MODER |= (mode << (pin*2));
}

void GPIO_Write(GPIO_TypeDef* port, uint8_t pin, bool val) {
	if (val) port->BSRR = (1<<pin);
	else port->BSRR = (1<<(pin+16));
}

void GPIO_SetAF(GPIO_TypeDef* port, uint8_t pin, uint8_t af) {
	if (pin >= 0 && pin <= 7) {
		port->AFR[0] &= ~(0xF << (4*pin));
		port->AFR[0] |= af << (4*pin);
	} else if (pin >= 8 && pin <= 15){
		port->AFR[1] &= ~(0xF << (4*(pin-8)));
		port->AFR[1] |= af << (4*(pin-8));
	}
}

void GPIO_EnableInterrupt(GPIO_TypeDef* port, uint8_t pin, uint8_t activation) {
	SYSCFG->EXTICR[pin/4] &= ~((0xF << (pin%4)*4));
	SYSCFG->EXTICR[pin/4] |= (((uint32_t)port - GPIOA_BASE) >> 10) << (pin%4)*4;

	EXTI->IMR |= 1 << pin;

	switch (activation) {
		case GPIO_INT_RISING:
			EXTI->RTSR |= 1 << pin;
			EXTI->FTSR &= ~(1 << pin);
			break;
		case GPIO_INT_FALLING:
			EXTI->RTSR &= ~(1 << pin);
			EXTI->FTSR |= 1 << pin;
			break;
		case GPIO_INT_BOTH:
			EXTI->RTSR |= 1 << pin;
			EXTI->FTSR |= 1 << pin;
			break;
		default:
			break;
	}

	switch (pin) {
		case 13:
			NVIC_EnableIRQ(EXTI15_10_IRQn);
			break;
		default:
			break;
	}

}
