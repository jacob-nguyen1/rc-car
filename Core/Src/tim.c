#include "tim.h"

static uint32_t TIM_GetClock(TIM_TypeDef *timer) {
    return 84000000;
}

void TIM_Init(TIM_TypeDef *timer, uint32_t freq, uint32_t per) {
    // Enable timer clock
    if (timer == TIM2) {
        RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    } else if (timer == TIM3) {
        RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
    } else if (timer == TIM4) {
        RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
    } else if (timer == TIM5) {
        RCC->APB1ENR |= RCC_APB1ENR_TIM5EN;
    }

    uint32_t timer_clk = TIM_GetClock(timer);
    uint32_t psc = (timer_clk / freq) - 1;

    timer->PSC = psc; // prescaler
    timer->ARR = per; // # of cycles before wraparound 
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
        default:
            while(1);
    }
}