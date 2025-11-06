#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>
#include <stdbool.h>
#include "stm32f4xx.h"

#define GPIO_MODE_INPUT  0b00
#define GPIO_MODE_OUTPUT 0b01
#define GPIO_MODE_AF     0b10
#define GPIO_MODE_ANALOG 0b11

void GPIO_SetMode(GPIO_TypeDef* port, uint8_t pin, uint8_t mode);
void GPIO_Write(GPIO_TypeDef* port, uint8_t pin, bool val);

#endif
