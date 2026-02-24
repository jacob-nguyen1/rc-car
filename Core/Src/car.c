#include "car.h"

void CarCommand(uint8_t comm) {
	printf("Executing 0x%02x | ", comm);
	switch (comm) {
		case 0xFF:
			printf("Error value\r\n");
			break;
		case 0x16:
			printf("0\r\n");
			break;
		case 0x0C:
			printf("1\r\n");
			break;
		case 0x18:
			printf("2\r\n");
			break;
		case 0x5E:
			printf("3\r\n");
			break;
		case 0x08:
			printf("4\r\n");
			break;
		case 0x1C:
			printf("5\r\n");
			break;
		case 0x5A:
			printf("6\r\n");
			break;
		case 0x42:
			printf("7\r\n");
			break;
		case 0x52:
			printf("8\r\n");
			break;
		case 0x4A:
			printf("9\r\n");
			break;
		case 0x45:
			printf("POWER\r\n");
			break;
		case 0x46:
			printf("VOL+\r\n");
			break;
		case 0x47:
			printf("FUNC/STOP\r\n");
			break;
		case 0x44:
			printf("LEFT\r\n");
			break;
		case 0x40:
			printf("PLAY/PAUSE\r\n");
			break;
		case 0x43:
			printf("RIGHT\r\n");
			break;
		case 0x07:
			printf("BACK\r\n");
			break;
		case 0x15:
			printf("VOL-\r\n");
			break;
		case 0x09:
			printf("FWD\r\n");
			break;
		case 0x19:
			printf("EQ\r\n");
			break;
		case 0x0d:
			printf("ST/REPT\r\n");
			break;
		default:
			printf("Unknown command: %X\r\n", comm);
			break;
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
