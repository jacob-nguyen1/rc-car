#include "stm32f446xx.h"
#include "gpio.h"
#include "pwm.h"
#include "car.h"
#include "tim.h"

// FRONT = L298N SIDE

int main(void) {
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


    TIM_Init(TIM2, 1000, 1000);
    TIM_Start(TIM2);

    while(1) {
    	if (!(GPIOC->IDR & GPIO_IDR_ID13)) {
    		CarForward();
    	} else {
    		CarStop();
    	}
    }
}
