#include "car.h"

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
