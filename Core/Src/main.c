#include "stm32f446xx.h"
#include "gpio.h"
#include "pwm.h"
#include "car.h"

// FRONT = L298N SIDE

int main(void) {
    // Enable IO clock
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN | RCC_AHB1ENR_GPIOCEN;
    // Enable TIM2 clock
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    // Enable SYSCFG clock
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    GPIO_SetMode(GPIOC, 13, GPIO_MODE_INPUT); // On-board USER pushbutton
    GPIO_EnableInterrupt(GPIOC, 13, GPIO_INT_FALLING);

    //GPIO_SetMode(GPIOA, 5, GPIO_MODE_OUTPUT); // On-board LED

    GPIO_SetMode(GPIOC, 0, GPIO_MODE_OUTPUT); // IN1
    GPIO_SetMode(GPIOC, 1, GPIO_MODE_OUTPUT); // IN2
    GPIO_SetMode(GPIOB, 0, GPIO_MODE_OUTPUT); // IN3
    GPIO_SetMode(GPIOA, 4, GPIO_MODE_OUTPUT); // IN4

    CarStop();

    GPIO_SetMode(GPIOA, 5, GPIO_MODE_AF);
    GPIO_SetAF(GPIOA, 5, 1);
    PWM_INIT(TIM2, 1, 1000);
    while(1) {
    	if (!(GPIOC->IDR & GPIO_IDR_ID13)) {
    		CarForward();
    	} else {
    		CarStop();
    	}
    }
}

void EXTI15_10_IRQHandler(void) {
	static bool h = 1;
	if (h) PWM_SetDutyCycle(TIM2, 1, 100);
	else PWM_SetDutyCycle(TIM2, 1, 0);
	h = !h;
}
