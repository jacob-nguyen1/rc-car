#ifndef PWM_H
#define PWM_H

#include <stdint.h>
#include <stdbool.h>
#include "stm32f4xx.h"

typedef enum {
    PWM_MODE_1 = 0x06U, // active when TIMx_CNT < TIMx_CCR1 else inactive
    PWM_MODE_2 = 0x07U  // inactive when TIMx_CNT < TIMx_CCR1 else active
} PWM_Mode_t;

typedef struct {
    uint32_t Prescaler;
    uint32_t Period;
    PWM_Mode_t Mode;
} PWM_InitTypeDef;

void PWM_Init(TIM_TypeDef *timer, uint8_t channel, PWM_InitTypeDef *init);
void PWM_SetDutyCycle(TIM_TypeDef *timer, uint8_t channel, uint8_t duty);

#endif
