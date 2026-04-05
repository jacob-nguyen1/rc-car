#include "tim.h"

uint32_t TIM_GetClock(TIM_TypeDef *timer) {
    // Allows dynamic prescaler calculations in main.c without hardcoding 16000000
    return 16000000;
}

void TIM_Init(TIM_TypeDef *timer, TIM_InitTypeDef *init) {
    // Enable timer clock
    if (timer == TIM2) {
        RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    } else if (timer == TIM3) {
        RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
    } else if (timer == TIM4) {
        RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
    } else if (timer == TIM5) {
        RCC->APB1ENR |= RCC_APB1ENR_TIM5EN;
    } else if (timer == TIM6) {
        RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;
    } else if (timer == TIM7) {
        RCC->APB1ENR |= RCC_APB1ENR_TIM7EN;
    }

    timer->PSC = init->Prescaler;
    timer->ARR = init->Period;
}

void TIM_Start(TIM_TypeDef *timer) {
    timer->CR1 |= TIM_CR1_CEN; 
}

void TIM_Stop(TIM_TypeDef *timer) {
    timer->CR1 &= ~TIM_CR1_CEN; 
}

void TIM_InputCapture_Init(TIM_TypeDef *timer, uint8_t channel, uint8_t edge) {
    switch (channel) {
        case 1:
            timer->CCMR1 &= ~TIM_CCMR1_CC1S;
            timer->CCMR1 |= TIM_CCMR1_CC1S_0;
            switch(edge) {
                case TIM_RISING_EDGE:
                    timer->CCER &= ~(TIM_CCER_CC1P | TIM_CCER_CC1NP);
                    break;
                case TIM_FALLING_EDGE:
                    timer->CCER &= ~(TIM_CCER_CC1P | TIM_CCER_CC1NP);
                    timer->CCER |= TIM_CCER_CC1P;
                    break;
                case TIM_BOTH_EDGES:
                    timer->CCER |= TIM_CCER_CC1P | TIM_CCER_CC1NP;
                    break;
                default:
                    while(1);
            }
            timer->CCER |= TIM_CCER_CC1E;
            break;
        case 2:
            timer->CCMR1 &= ~TIM_CCMR1_CC2S;
            timer->CCMR1 |= TIM_CCMR1_CC2S_1;
            switch(edge) {
                case TIM_RISING_EDGE:
                    timer->CCER &= ~(TIM_CCER_CC2P | TIM_CCER_CC2NP);
                    break;
                case TIM_FALLING_EDGE:
                    timer->CCER &= ~(TIM_CCER_CC2P | TIM_CCER_CC2NP);
                    timer->CCER |= TIM_CCER_CC2P;
                    break;
                case TIM_BOTH_EDGES:
                    timer->CCER |= TIM_CCER_CC2P | TIM_CCER_CC2NP;
                    break;
                default:
                    while(1);
            }
            timer->CCER |= TIM_CCER_CC2E;
            break;
        default:
            while(1);
    }
}