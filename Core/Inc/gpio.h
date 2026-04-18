#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>
#include <stdbool.h>
#include "stm32f446xx.h"

typedef enum {
    G_GPIO_MODE_INPUT  = 0x00U,
    G_GPIO_MODE_OUTPUT = 0x01U,
    G_GPIO_MODE_AF     = 0x02U,
    G_GPIO_MODE_ANALOG = 0x03U
} GPIO_Mode_t;

typedef enum {
    G_GPIO_INT_RISING  = 0x00U,
    G_GPIO_INT_FALLING = 0x01U,
    G_GPIO_INT_BOTH    = 0x02U
} GPIO_Edge_t;

typedef enum {
    G_GPIO_PULL_NONE = 0x00U,
    G_GPIO_PULL_UP   = 0x01U,
    G_GPIO_PULL_DOWN = 0x02U
} GPIO_Pull_t;

void GPIO_SetMode(GPIO_TypeDef* port, uint8_t pin, GPIO_Mode_t mode);
void GPIO_Write(GPIO_TypeDef* port, uint8_t pin, bool val);
void GPIO_SetAF(GPIO_TypeDef* port, uint8_t pin, uint8_t af);
void GPIO_SetPull(GPIO_TypeDef* port, uint8_t pin, GPIO_Pull_t pull);
void GPIO_EnableInterrupt(GPIO_TypeDef* port, uint8_t pin, GPIO_Edge_t edge);

#endif
