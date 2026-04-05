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

static void MX_GPIO_Init(void) {
    // Enable IO clock for all active ports
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN | RCC_AHB1ENR_GPIOCEN;

    // Configure PC13 as On-board USER pushbutton with Pull-Up
    GPIO_SetMode(GPIOC, 13, GPIO_MODE_INPUT);
    GPIO_SetPull(GPIOC, 13, GPIO_PULL_UP);

    // Configure PA5 as On-board LED
    GPIO_SetMode(GPIOA, 5, GPIO_MODE_OUTPUT);

    // Configure PA6 for IR Receiver Input
    GPIO_SetMode(GPIOA, 6, GPIO_MODE_AF);
    GPIO_SetAF(GPIOA, 6, 2);

    // Configure PB6, PB7, PB8, PB9 for Motor PWM (TIM4)
    GPIO_SetMode(GPIOB, 6, GPIO_MODE_AF);
    GPIO_SetMode(GPIOB, 7, GPIO_MODE_AF);
    GPIO_SetMode(GPIOB, 8, GPIO_MODE_AF);
    GPIO_SetMode(GPIOB, 9, GPIO_MODE_AF);
    GPIO_SetAF(GPIOB, 6, 2);
    GPIO_SetAF(GPIOB, 7, 2);
    GPIO_SetAF(GPIOB, 8, 2);
    GPIO_SetAF(GPIOB, 9, 2);
}

static void MX_TIM4_Init(void) {
    // Enable the Motor PWM Timer (TIM4)
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
}

static void MX_TIM3_Init(void) {
    // IR Receiver Input Capture timer (1MHz counting)
    TIM_InitTypeDef tim3_init = { .Prescaler = (TIM_GetClock(TIM3) / 1000000) - 1, .Period = 65535 };
    TIM_Init(TIM3, &tim3_init);
    
    // Capture both edges to timestamp IR pulses
    TIM_InputCapture_Init(TIM3, 1, TIM_BOTH_EDGES);
    TIM_InputCapture_Init(TIM3, 2, TIM_BOTH_EDGES);
    
    // Enable timer interrupts for timeouts
    TIM3->DIER |= TIM_DIER_CC2IE;
    NVIC_EnableIRQ(TIM3_IRQn);
    TIM_Start(TIM3);
}

static void MX_TIM6_Init(void) {
    // Timeout timer for decoding instruction (10ms)
    TIM_InitTypeDef tim6_init = { .Prescaler = (TIM_GetClock(TIM6) / 1000000) - 1, .Period = 10000 };
    TIM_Init(TIM6, &tim6_init);
    TIM6->DIER |= TIM_DIER_UIE;
    NVIC_EnableIRQ(TIM6_DAC_IRQn);
}

static void MX_TIM7_Init(void) {
    // Timeout timer for repeat instructions (200ms)
    TIM_InitTypeDef tim7_init = { .Prescaler = (TIM_GetClock(TIM7) / 100000) - 1, .Period = 20000 };
    TIM_Init(TIM7, &tim7_init);
    TIM7->DIER |= TIM_DIER_UIE;
    NVIC_EnableIRQ(TIM7_IRQn);
}

static void MX_DMA_Init(void) {
    // Initialize DMA strictly for TIM3 CH1 transfers
    DMA_TIM3_CH1_Init(timestamps, BUF_SIZE);
}

int main(void) {
    // Initial hardware clock/debug setup
    Debug_Init();
    
    // Core Peripheral Initialization
    MX_GPIO_Init();
    MX_TIM4_Init();
    MX_TIM3_Init();
    MX_DMA_Init();
    MX_TIM6_Init();
    MX_TIM7_Init();
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