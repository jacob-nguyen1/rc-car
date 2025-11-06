#include "stm32f446xx.h"
#include "gpio.h"

typedef enum {
	LEFT,
	RIGHT
} MotorSide;

typedef enum {
	FWD,
	BCK,
	STOP
} MotorDir;

// FRONT = L298N SIDE

void SetWheelDir(MotorSide side, MotorDir dir);

void CarForward();
void CarStop();

int main(void) {
    // Enable IO clock
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN | RCC_AHB1ENR_GPIOCEN;

    GPIO_SetMode(GPIOC, 13, GPIO_MODE_INPUT); // On-board USER pushbutton

    GPIO_SetMode(GPIOA, 5, GPIO_MODE_OUTPUT); // On-board LED

    GPIO_SetMode(GPIOC, 0, GPIO_MODE_OUTPUT); // IN1
    GPIO_SetMode(GPIOC, 1, GPIO_MODE_OUTPUT); // IN2
    GPIO_SetMode(GPIOB, 0, GPIO_MODE_OUTPUT); // IN3
    GPIO_SetMode(GPIOA, 4, GPIO_MODE_OUTPUT); // IN4

    CarStop();

    while(1) {
    	if (!(GPIOC->IDR & GPIO_IDR_ID13)) {
    		GPIO_Write(GPIOA, 5, 1);
    		CarForward();
    	} else {
    		GPIO_Write(GPIOA, 5, 0);
    		CarStop();
    	}

    }
}

void SetWheelDir(MotorSide side, MotorDir dir) {
	switch(side) {
		case LEFT:
			switch(dir) {
				case FWD:
					GPIO_Write(GPIOC, 0, 0);
					GPIO_Write(GPIOC, 1, 1);
					break;
				case BCK:
					GPIO_Write(GPIOC, 0, 1);
					GPIO_Write(GPIOC, 1, 0);
					break;
				case STOP:
					GPIO_Write(GPIOC, 0, 0);
					GPIO_Write(GPIOC, 1, 0);
					break;
			}
			break;
		case RIGHT:
			switch(dir) {
				case FWD:
					GPIO_Write(GPIOB, 0, 0);
					GPIO_Write(GPIOA, 4, 1);
					break;
				case BCK:
					GPIO_Write(GPIOB, 0, 1);
					GPIO_Write(GPIOA, 4, 0);
					break;
				case STOP:
					GPIO_Write(GPIOB, 0, 0);
					GPIO_Write(GPIOA, 4, 0);
					break;
			}
			break;
	}
}

void CarForward() {
	SetWheelDir(LEFT, FWD);
	SetWheelDir(RIGHT, FWD);
}

void CarStop() {
	SetWheelDir(LEFT, STOP);
	SetWheelDir(RIGHT, STOP);
}
