#include "car.h"
#include "pwm.h"

static CarState current_state = CAR_STATE_STOPPED;
static uint8_t car_speed = 77;

void CarSetSpeed(uint8_t speed) {
	car_speed = speed;
}

uint8_t CarGetSpeed(void) {
	return car_speed;
}

CarState CarGetState(void) {
	return current_state;
}

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
			CarSetSpeed(11);
			break;
		case 0x18:
			printf("2\r\n");
			CarSetSpeed(22);
			break;
		case 0x5E:
			printf("3\r\n");
			CarSetSpeed(33);
			break;
		case 0x08:
			printf("4\r\n");
			CarSetSpeed(44);
			break;
		case 0x1C:
			printf("5\r\n");
			CarSetSpeed(55);
			break;
		case 0x5A:
			printf("6\r\n");
			CarSetSpeed(66);
			break;
		case 0x42:
			printf("7\r\n");
			CarSetSpeed(77);
			break;
		case 0x52:
			printf("8\r\n");
			CarSetSpeed(88);
			break;
		case 0x4A:
			printf("9\r\n");
			CarSetSpeed(100);
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
			CarSetState(CAR_STATE_PIVOTLEFT);
			break;
		case 0x40:
			printf("PLAY/PAUSE\r\n");
			break;
		case 0x43:
			printf("RIGHT\r\n");
			CarSetState(CAR_STATE_PIVOTRIGHT);
			break;
		case 0x07:
			printf("BACK\r\n");
			CarSetState(CAR_STATE_REVERSE);
			break;
		case 0x15:
			printf("VOL-\r\n");
			break;
		case 0x09:
			printf("FWD\r\n");
			CarSetState(CAR_STATE_FORWARD);
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

void SetWheelDir(MotorSide side, MotorDir dir, int32_t speed) {
	if (speed < 0) speed = 0;
	if (speed > 100) speed = 100;
	switch(side) {
		case MOTOR_SIDE_LEFT:
			switch(dir) {
				case MOTOR_DIR_FWD:
					PWM_SetDutyCycle(TIM4, 1, 0);
					PWM_SetDutyCycle(TIM4, 2, speed);
					break;
				case MOTOR_DIR_BCK:
					PWM_SetDutyCycle(TIM4, 1, speed);
					PWM_SetDutyCycle(TIM4, 2, 0);
					break;
				case MOTOR_DIR_STOP:
					PWM_SetDutyCycle(TIM4, 1, 0);
					PWM_SetDutyCycle(TIM4, 2, 0);
					break;
			}
			break;
		case MOTOR_SIDE_RIGHT:
			switch(dir) {
				case MOTOR_DIR_FWD:
					PWM_SetDutyCycle(TIM4, 3, 0);
					PWM_SetDutyCycle(TIM4, 4, speed);
					break;
				case MOTOR_DIR_BCK:
					PWM_SetDutyCycle(TIM4, 3, speed);
					PWM_SetDutyCycle(TIM4, 4, 0);
					break;
				case MOTOR_DIR_STOP:
					PWM_SetDutyCycle(TIM4, 3, 0);
					PWM_SetDutyCycle(TIM4, 4, 0);
					break;
			}
			break;
	}
}

void CarSetState(CarState state) {
	current_state = state;
	switch (state) {
		case CAR_STATE_FORWARD:
			SetWheelDir(MOTOR_SIDE_LEFT, MOTOR_DIR_FWD, car_speed);
			SetWheelDir(MOTOR_SIDE_RIGHT, MOTOR_DIR_FWD, car_speed);
			break;
		case CAR_STATE_REVERSE:
			SetWheelDir(MOTOR_SIDE_LEFT, MOTOR_DIR_BCK, car_speed);
			SetWheelDir(MOTOR_SIDE_RIGHT, MOTOR_DIR_BCK, car_speed);
			break;
		case CAR_STATE_PIVOTLEFT:
			SetWheelDir(MOTOR_SIDE_LEFT, MOTOR_DIR_BCK, car_speed);
			SetWheelDir(MOTOR_SIDE_RIGHT, MOTOR_DIR_FWD, car_speed);
			break;
		case CAR_STATE_PIVOTRIGHT:
			SetWheelDir(MOTOR_SIDE_LEFT, MOTOR_DIR_FWD, car_speed);
			SetWheelDir(MOTOR_SIDE_RIGHT, MOTOR_DIR_BCK, car_speed);
			break;
		case CAR_STATE_STOPPED:
			SetWheelDir(MOTOR_SIDE_LEFT, MOTOR_DIR_STOP, car_speed);
			SetWheelDir(MOTOR_SIDE_RIGHT, MOTOR_DIR_STOP, car_speed);
			printf("CAR STOP\r\n");
			break;
		default:
			SetWheelDir(MOTOR_SIDE_LEFT, MOTOR_DIR_STOP, car_speed);
			SetWheelDir(MOTOR_SIDE_RIGHT, MOTOR_DIR_STOP, car_speed);
			break;
	}
}
