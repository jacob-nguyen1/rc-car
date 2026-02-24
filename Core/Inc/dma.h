#ifndef DMA_H
#define DMA_H

#include <stdint.h>
#include "stm32f446xx.h"

void DMA_TIM3_CH1_Init(uint16_t* buffer, uint16_t size);

#endif