#include "stm32f446xx.h"
#include "gpio.h"
#include "pwm.h"
#include "car.h"
#include "tim.h"
#include "debug.h"
#include "dma.h"
#include "ir.h"

#include <stdio.h>
#include <stdbool.h>

// FRONT = L298N SIDE

#define BUF_SIZE 66

// Buffer for timer input capture timestamps
uint16_t timestamps[BUF_SIZE];

volatile bool dma_complete = false;
volatile bool disable_dma = false;

volatile bool isr_tim6 = false;
volatile bool isr_tim7 = false;

int main(void) {
    { // config
    Debug_Init();
    // Enable IO clock
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN | RCC_AHB1ENR_GPIOCEN;

    GPIO_SetMode(GPIOC, 13, GPIO_MODE_INPUT); // On-board USER pushbutton
    GPIO_SetPull(GPIOC, 13, GPIO_PULL_UP);

    GPIO_SetMode(GPIOA, 5, GPIO_MODE_OUTPUT); // On-board LED

    GPIO_SetMode(GPIOB, 6, GPIO_MODE_AF); // IN1 (TIM4_CH1) - Left
    GPIO_SetMode(GPIOB, 7, GPIO_MODE_AF); // IN2 (TIM4_CH2) - Left
    GPIO_SetMode(GPIOB, 8, GPIO_MODE_AF); // IN3 (TIM4_CH3) - Right
    GPIO_SetMode(GPIOB, 9, GPIO_MODE_AF); // IN4 (TIM4_CH4) - Right

    GPIO_SetAF(GPIOB, 6, 2);
    GPIO_SetAF(GPIOB, 7, 2);
    GPIO_SetAF(GPIOB, 8, 2);
    GPIO_SetAF(GPIOB, 9, 2);

    RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;

    PWM_InitTypeDef pwm_init = { 
        .Prescaler = 0, 
        .Period = (TIM_GetClock(TIM4) / 20000) - 1, 
        .Mode = PWM_MODE_1 
    };

    PWM_Init(TIM4, 1, &pwm_init);
    PWM_Init(TIM4, 2, &pwm_init);
    PWM_Init(TIM4, 3, &pwm_init);
    PWM_Init(TIM4, 4, &pwm_init);

    // input capture timer 
    TIM_InitTypeDef tim3_init = { .Prescaler = (TIM_GetClock(TIM3) / 1000000) - 1, .Period = 65535 };
    TIM_Init(TIM3, &tim3_init);
    TIM_InputCapture_Init(TIM3, 1, TIM_BOTH_EDGES);
    TIM_InputCapture_Init(TIM3, 2, TIM_BOTH_EDGES);
    TIM3->DIER |= TIM_DIER_CC2IE; // used for timeout
    NVIC_EnableIRQ(TIM3_IRQn);
    TIM_Start(TIM3);

    // connect to PIN_A6 to IR receiver
    GPIO_SetMode(GPIOA, 6, GPIO_MODE_AF);
    GPIO_SetAF(GPIOA, 6, 2);

    DMA_TIM3_CH1_Init(timestamps, BUF_SIZE);

    // timeout for decoding instruction
    TIM_InitTypeDef tim6_init = { .Prescaler = (TIM_GetClock(TIM6) / 1000000) - 1, .Period = 10000 }; // 10ms
    TIM_Init(TIM6, &tim6_init);
    TIM6->DIER |= TIM_DIER_UIE;
    NVIC_EnableIRQ(TIM6_DAC_IRQn);

    // timeout for repeat instructions
    TIM_InitTypeDef tim7_init = { .Prescaler = (TIM_GetClock(TIM7) / 100000) - 1, .Period = 20000 }; // 200ms
    TIM_Init(TIM7, &tim7_init);
    TIM7->DIER |= TIM_DIER_UIE;
    NVIC_EnableIRQ(TIM7_IRQn);
    }
    while(1) {
        if (dma_complete) {
            uint8_t comm = IR_Decode(timestamps, BUF_SIZE);
            CarCommand(comm);

            if (comm != 0xFF) GPIO_Write(GPIOA, 5, 1);

            DMA1_Stream4->NDTR = BUF_SIZE;
            DMA1_Stream4->M0AR = (uint32_t)timestamps;
            DMA1_Stream4->CR |= DMA_SxCR_EN;
            dma_complete = false;
        }
        if (isr_tim6) {
            printf("H");
            fflush(stdout);
            isr_tim6 = false;
        }
        if (isr_tim7) {
            printf("S\r\n");
            isr_tim7 = false;
        }
        //button
    	if (!(GPIOC->IDR & GPIO_IDR_ID13)) { // Pressed
           //GPIO_Write(GPIOA, 5, 0); 
    	} else { // Un-pressed
           //GPIO_Write(GPIOA, 5, 1); 
    	}
    }
}

void DMA1_Stream4_IRQHandler(void) {
    if (DMA1->HISR & DMA_HISR_TCIF4) {
        DMA1->HIFCR |= DMA_HIFCR_CTCIF4;
        dma_complete = true;
    }
}

void TIM3_IRQHandler(void) {
    if (TIM3->SR & TIM_SR_CC2IF) {
        TIM3->SR &= ~TIM_SR_CC2IF;
        TIM6->CNT = 0;
        TIM_Start(TIM6);
        TIM7->CNT = 0;
        TIM_Start(TIM7);
    }
}

void TIM6_DAC_IRQHandler(void) {
    isr_tim6 = true;
    if (TIM6->SR & TIM_SR_UIF) {
        TIM_Stop(TIM6);
        TIM6->SR &= ~TIM_SR_UIF;

        // clear buffer
        for (int i=0; i<BUF_SIZE; i++) {
            timestamps[i] = 0;
        }

        // reset DMA
        // Disable stream before config
        DMA1_Stream4->CR &= ~DMA_SxCR_EN;
        while (DMA1_Stream4->CR & DMA_SxCR_EN);

        DMA1_Stream4->NDTR = BUF_SIZE;
        DMA1_Stream4->M0AR = (uint32_t)timestamps;
        // Clear ALL flags for stream 4 before enabling
        DMA1->HIFCR = DMA_HIFCR_CTCIF4 | DMA_HIFCR_CHTIF4 | 
                        DMA_HIFCR_CTEIF4 | DMA_HIFCR_CDMEIF4 | DMA_HIFCR_CFEIF4;

        // Enable
        DMA1_Stream4->CR |= DMA_SxCR_EN;

    }
}

void TIM7_IRQHandler(void) {
    isr_tim7 = true;
    if (TIM7->SR & TIM_SR_UIF) {
        TIM_Stop(TIM7);
        TIM7->SR &= ~TIM_SR_UIF;
        CarSetState(CAR_STATE_STOPPED);
        GPIO_Write(GPIOA, 5, 0);
    }
}