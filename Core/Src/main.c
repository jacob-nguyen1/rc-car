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

int main(void) {
    Debug_Init();
    // Enable IO clock
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN | RCC_AHB1ENR_GPIOCEN;

    GPIO_SetMode(GPIOC, 13, GPIO_MODE_INPUT); // On-board USER pushbutton
    GPIO_SetPull(GPIOC, 13, GPIO_PULL_UP);

    GPIO_SetMode(GPIOA, 5, GPIO_MODE_OUTPUT); // On-board LED

    GPIO_SetMode(GPIOC, 0, GPIO_MODE_OUTPUT); // IN1
    GPIO_SetMode(GPIOC, 1, GPIO_MODE_OUTPUT); // IN2
    GPIO_SetMode(GPIOB, 0, GPIO_MODE_OUTPUT); // IN3
    GPIO_SetMode(GPIOA, 4, GPIO_MODE_OUTPUT); // IN4

    CarStop();
    
    // input capture timer
    TIM_Init(TIM3, 1000000, 65535);
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
    TIM_Init(TIM4, 1000000, 10000); // 10ms
    TIM4->DIER |= TIM_DIER_UIE;
    NVIC_EnableIRQ(TIM4_IRQn);

    // timeout for repeat instructions
    TIM_Init(TIM5, 100000, 10000); // 150ms
    TIM5->DIER |= TIM_DIER_UIE;
    NVIC_EnableIRQ(TIM5_IRQn);

    printf("Starting...\r\n");
    while(1) {
        if (dma_complete) {
            uint8_t comm = IR_Decode(timestamps, BUF_SIZE);
            CarCommand(comm);

            DMA1_Stream4->NDTR = BUF_SIZE;
            DMA1_Stream4->M0AR = (uint32_t)timestamps;
            DMA1_Stream4->CR |= DMA_SxCR_EN;
            dma_complete = false;
        }
        //button
    	if (!(GPIOC->IDR & GPIO_IDR_ID13)) { // Pressed
    		//CarForward();
    	} else { // Un-pressed
    		//CarStop();
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
        TIM4->CNT = 0;
        TIM_Start(TIM4);
        TIM5->CNT = 0;
        TIM_Start(TIM5);
    }
}

void TIM4_IRQHandler(void) {
    if (TIM4->SR & TIM_SR_UIF) {
        TIM_Stop(TIM4);
        TIM4->SR &= ~TIM_SR_UIF;

        // reset DMA
        // Disable stream before config
        DMA1_Stream4->CR &= ~DMA_SxCR_EN;
        while (DMA1_Stream4->CR & DMA_SxCR_EN);

        // Clear ALL flags for stream 4 before enabling
        DMA1->HIFCR = DMA_HIFCR_CTCIF4 | DMA_HIFCR_CHTIF4 | 
                        DMA_HIFCR_CTEIF4 | DMA_HIFCR_CDMEIF4 | DMA_HIFCR_CFEIF4;

        // Enable
        DMA1_Stream4->CR |= DMA_SxCR_EN;

        // clear buffer
        for (int i=0; i<BUF_SIZE; i++) {
            timestamps[i] = 0;
        }
    }
}

void TIM5_IRQHandler(void) {
    if (TIM5->SR & TIM_SR_UIF) {
        TIM_Stop(TIM5);
        TIM5->SR &= ~TIM_SR_UIF;
    }
}