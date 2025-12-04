#include "pwm.h"

void PWM_INIT(TIM_TypeDef* timer, uint8_t channel, uint16_t freq) {
	timer->ARR = 16000000UL / freq - 1;
	switch (channel) {
		case 1:
			timer->CCMR1 &= ~(TIM_CCMR1_OC1M);
			timer->CCMR1 |= (PWM_MODE1 << TIM_CCMR1_OC1M_Pos);
			timer->CCER |= TIM_CCER_CC1E;
			timer->CCR1 = 8000;
			break;
		case 2:
			timer->CCMR1 &= ~(TIM_CCMR1_OC2M);
			timer->CCMR1 |= (PWM_MODE1 << TIM_CCMR1_OC2M_Pos);
			timer->CCER |= TIM_CCER_CC2E;
			timer->CCR2 = 0;
			break;
		case 3:
			timer->CCMR2 &= ~(TIM_CCMR2_OC3M);
			timer->CCMR2 |= (PWM_MODE1 << TIM_CCMR2_OC3M_Pos);
			timer->CCER |= TIM_CCER_CC3E;
			timer->CCR3 = 0;
			break;
		case 4:
			timer->CCMR2 &= ~(TIM_CCMR2_OC4M);
			timer->CCMR2 |= (PWM_MODE1 << TIM_CCMR2_OC4M_Pos);
			timer->CCER |= TIM_CCER_CC4E;
			timer->CCR4 = 0;
			break;
		default:
			while(1);
	}
	timer->CR1 |= TIM_CR1_CEN | TIM_CR1_ARPE;
}
