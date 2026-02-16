#include "stm32f446xx.h"
#include "gpio.h"
#include "pwm.h"
#include "car.h"
#include "tim.h"
#include "debug.h"

// FRONT = L298N SIDE

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
    
    TIM_Init(TIM3, 1000000, 1000);
    TIM_Start(TIM3);

    //temp
    TIM_InputCapture_Init(TIM3, 1, 0);
    GPIO_SetMode(GPIOA, 6, GPIO_MODE_AF);
    GPIO_SetAF(GPIOA, 6, 2);

    uint16_t timestamps[40];
    uint8_t index = 0;

    while(1) {
        if (TIM3->SR & TIM_SR_CC1IF) {
            if (index < 40) {
                timestamps[index++] = TIM3->CCR1;
            }
            TIM3->SR &= ~TIM_SR_CC1IF;  // Clear flag
        }
        
        // Print after capture completes
        if (index >= 40) {
            for(int i = 0; i < 40; i++) {
                printf("[%d] %u\r\n", i, timestamps[i]);
            }
            printf("--- DONE, press button again ---\r\n");
            index = 0;  // Reset
        }
        //button
    	if (!(GPIOC->IDR & GPIO_IDR_ID13)) {
    		CarForward();
    	} else {
    		CarStop();
    	}
    }
}
