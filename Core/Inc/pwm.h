#ifndef PWM_H
#define PWM_H

#include <stdint.h>
#include <stdbool.h>
#include "stm32f4xx.h"

#define PWM_MODE1 0b110 // active when TIMx_CNT<TIMx_CCR1 else inactive
#define PWM_MODE2 0b111 // inactive when TIMx_CNT<TIMx_CCR1 else active

void PWM_INIT(TIM_TypeDef* tim, uint8_t ch, uint16_t freq);

#endif
