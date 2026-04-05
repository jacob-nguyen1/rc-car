#ifndef TIM_H
#define TIM_H

#include <stdint.h>
#include <stdbool.h>
#include "stm32f4xx.h"

#define TIM_MAX_16BIT 65535
#define TIM_MAX_32BIT 4294967295

typedef struct {
    uint32_t Prescaler;
    uint32_t Period;
} TIM_InitTypeDef;

uint32_t TIM_GetClock(TIM_TypeDef *timer);

void TIM_Init(TIM_TypeDef *timer, TIM_InitTypeDef *init);
void TIM_Start(TIM_TypeDef *timer);
void TIM_Stop(TIM_TypeDef *timer);

#define TIM_RISING_EDGE  0
#define TIM_FALLING_EDGE 1  
#define TIM_BOTH_EDGES   2

void TIM_InputCapture_Init(TIM_TypeDef *timer, uint8_t channel, uint8_t edge);

#endif