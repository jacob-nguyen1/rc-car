#include "gpio.h"

void GPIO_SetMode(GPIO_TypeDef* port, uint8_t pin, uint8_t mode) {
	port->MODER &= ~(0b11 << (pin*2));
	port->MODER |= (mode << (pin*2));
}

void GPIO_Write(GPIO_TypeDef* port, uint8_t pin, bool val) {
	if (val) port->BSRR = (1<<pin);
	else port->BSRR = (1<<(pin+16));
}
