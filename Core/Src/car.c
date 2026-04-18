#include "car.h"
#include "pwm.h"
#include "config.h"
#include "gpio.h"

static CarState current_state = CAR_STATE_STOPPED;
static bool closed_loop_en = true;

// The period goal for the PID loop (closed loop)
static int32_t target_period = 99999999; 

// Rough proxy for Reverse/Pivot (open loop)
static uint8_t base_pwm = 77; 

void CarSetRPM(int32_t target_rpm) {
	if (target_rpm == 0) {
		target_period = 99999999;
		base_pwm = 0;
	} else {
		target_period = RPM_TO_PERIOD_CONST / target_rpm;
		
		if (current_state == CAR_STATE_PIVOTLEFT || current_state == CAR_STATE_PIVOTRIGHT) {
			base_pwm = (target_rpm * 100) / MAX_RPM_PIVOT;
			if (base_pwm > PWM_CEILING) base_pwm = PWM_CEILING;
			if (base_pwm < PWM_FLOOR_PIVOT) base_pwm = PWM_FLOOR_PIVOT;
		} else {
			base_pwm = (target_rpm * 100) / MAX_RPM_LINEAR;
			if (base_pwm > PWM_CEILING) base_pwm = PWM_CEILING;
			if (base_pwm < PWM_FLOOR_LINEAR) base_pwm = PWM_FLOOR_LINEAR;
		}
	}
}

int32_t CarGetTargetPeriod(void) {
	return target_period;
}

uint8_t CarGetBasePWM(void) {
    return base_pwm;
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
			CarSetRPM(40);
			break;
		case 0x18:
			printf("2\r\n");
			CarSetRPM(50);
			break;
		case 0x5E:
			printf("3\r\n");
			CarSetRPM(60);
			break;
		case 0x08:
			printf("4\r\n");
			CarSetRPM(70);
			break;
		case 0x1C:
			printf("5\r\n");
			CarSetRPM(80);
			break;
		case 0x5A:
			printf("6\r\n");
			CarSetRPM(90);
			break;
		case 0x42:
			printf("7\r\n");
			CarSetRPM(100);
			break;
		case 0x52:
			printf("8\r\n");
			CarSetRPM(110);
			break;
		case 0x4A:
			printf("9\r\n");
			CarSetRPM(120);
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
			CarToggleClosedLoop();
			break;
		default:
			printf("Unknown command: %X\r\n", comm);
			break;
	} 
}

bool CarIsClosedLoop(void) {
	return closed_loop_en;
}

void CarToggleClosedLoop(void) {
	closed_loop_en = !closed_loop_en;
	CarUpdateLED(); // Refresh LED immediately to show mode change
}

void CarUpdateLED(void) {
	bool moving = (current_state != CAR_STATE_STOPPED);
	if (closed_loop_en) {
		// Active High: ON when moving
		GPIO_Write(GPIOA, 5, moving ? 1 : 0);
	} else {
		// Active Low: ON when stopped
		GPIO_Write(GPIOA, 5, moving ? 0 : 1);
	}
}

void SetWheelDir(MotorSide side, MotorDir dir, int32_t speed) {
	if (speed < PWM_FLOOR_LINEAR) speed = PWM_FLOOR_LINEAR;
	if (speed > PWM_CEILING) speed = PWM_CEILING;
	switch(side) {
		case MOTOR_SIDE_LEFT:
			switch(dir) {
				case MOTOR_DIR_FWD:
					PWM_SetDutyCycle(TIM4, 1, 100 - speed);
					PWM_SetDutyCycle(TIM4, 2, 100);
					break;
				case MOTOR_DIR_BCK:
					PWM_SetDutyCycle(TIM4, 1, 100);
					PWM_SetDutyCycle(TIM4, 2, 100 - speed);
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
					PWM_SetDutyCycle(TIM4, 3, 100 - speed);
					PWM_SetDutyCycle(TIM4, 4, 100);
					break;
				case MOTOR_DIR_BCK:
					PWM_SetDutyCycle(TIM4, 3, 100);
					PWM_SetDutyCycle(TIM4, 4, 100 - speed);
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
	CarUpdateLED();
	switch (state) {
		case CAR_STATE_FORWARD:
			break;
		case CAR_STATE_REVERSE:
			SetWheelDir(MOTOR_SIDE_LEFT, MOTOR_DIR_BCK, base_pwm);
			SetWheelDir(MOTOR_SIDE_RIGHT, MOTOR_DIR_BCK, base_pwm);
			break;
		case CAR_STATE_PIVOTLEFT:
			SetWheelDir(MOTOR_SIDE_LEFT, MOTOR_DIR_BCK, base_pwm);
			SetWheelDir(MOTOR_SIDE_RIGHT, MOTOR_DIR_FWD, base_pwm);
			break;
		case CAR_STATE_PIVOTRIGHT:
			SetWheelDir(MOTOR_SIDE_LEFT, MOTOR_DIR_FWD, base_pwm);
			SetWheelDir(MOTOR_SIDE_RIGHT, MOTOR_DIR_BCK, base_pwm);
			break;
		case CAR_STATE_STOPPED:
			SetWheelDir(MOTOR_SIDE_LEFT, MOTOR_DIR_STOP, base_pwm);
			SetWheelDir(MOTOR_SIDE_RIGHT, MOTOR_DIR_STOP, base_pwm);
			printf("CAR STOP\r\n");
			break;
		default:
			SetWheelDir(MOTOR_SIDE_LEFT, MOTOR_DIR_STOP, base_pwm);
			SetWheelDir(MOTOR_SIDE_RIGHT, MOTOR_DIR_STOP, base_pwm);
			break;
	}
}
