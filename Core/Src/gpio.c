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
