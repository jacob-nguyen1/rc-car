#include "stm32f446xx.h"
#include "gpio.h"
#include "pwm.h"
#include "car.h"
#include "tim.h"
#include "debug.h"
#include "dma.h"

#include <stdio.h>
#include <stdbool.h>

// FRONT = L298N SIDE

#define SIZE 66

// Buffer for timer input capture timestamps
uint16_t timestamps[SIZE];

volatile bool dma_complete = false;

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
    
    TIM_Init(TIM3, 1000000, 65535);
    TIM_InputCapture_Init(TIM3, 1, TIM_BOTH_EDGES);
    GPIO_SetMode(GPIOA, 6, GPIO_MODE_AF);
    GPIO_SetAF(GPIOA, 6, 2);
    TIM_Start(TIM3);

    DMA_TIM3_CH1_Init(timestamps, SIZE);


    //timeout
    TIM_Init(TIM4, 1000000, 50000);
    TIM4->DIER |= TIM_DIER_UIE;
    NVIC_EnableIRQ(TIM4_IRQn);
    TIM4->CR1 |= TIM_CR1_OPM;
    TIM_Start(TIM4);
    
    printf("Starting...\r\n");
    while(1) {
        if (dma_complete) {
            printf("DMA worked!\r\n");
            uint16_t pulses[SIZE-1];
            int p = 0;
            for (int i = 1; i < SIZE; i++) {
                uint16_t t1 = timestamps[i-1];
                uint16_t t2 = timestamps[i];
                
                uint16_t diff;
                if (t2 >= t1) {
                    diff = t2 - t1;  // Normal case
                } else {
                    diff = (65536 - t1) + t2;  // Wrapped
                }
                
                pulses[p++] = diff;
            } 

            for (int i=0; i<SIZE-1; i++) {
                printf("%d ", pulses[i]);
            } printf("\r\n");


            int start;
            // find 9000 4500
            for (int i=1; i<SIZE-1; i++) {
                if (pulses[i-1] > 8500 && pulses[i-1] < 9500 && pulses[i] > 4000 && pulses[i] < 5000) {
                    start = i+1;
                    printf("Found start: %d\r\n", start);
                }
            }

            for (int i=start; i<SIZE-1; i+=2) {
                printf("%d", pulses[i+1]<1000 ? 0 : 1);
            } printf("\r\n");
            if (start > 15) {
                printf("Cooked\r\n");
            } else {
                for (int i=start+32; i<start+48; i+=2) {
                    printf("%d", pulses[i+1]<1000 ? 0 : 1);
                } printf("\r\n");
            }

            DMA1_Stream4->NDTR = SIZE;
            DMA1_Stream4->M0AR = (uint32_t)timestamps;
            DMA1_Stream4->CR |= DMA_SxCR_EN;
            dma_complete = false;
        }
        //button
    	if (!(GPIOC->IDR & GPIO_IDR_ID13)) { // Pressed
    		CarForward();
    	} else { // Un-pressed
    		CarStop();
    	}
    }
}

void DMA1_Stream4_IRQHandler(void) {
    if (DMA1->HISR & DMA_HISR_TCIF4) {
        TIM4->CNT = 0;

        DMA1->HIFCR |= DMA_HIFCR_CTCIF4;
        dma_complete = true;
    }
}

void TIM4_IRQHandler(void) {
    if (TIM4->SR & TIM_SR_UIF) {
        TIM4->SR &= ~TIM_SR_UIF;

        TIM_Stop(TIM4);
        
        DMA1_Stream4->CR &= ~DMA_SxCR_EN;
        while (DMA1_Stream4->CR & DMA_SxCR_EN);
        DMA1_Stream4->NDTR = SIZE;
        DMA1_Stream4->M0AR = (uint32_t)timestamps;
        DMA1_Stream4->CR |= DMA_SxCR_EN;
    }
}