#ifndef CAR_H
#define CAR_H

#include <stdint.h>
#include <stdbool.h>
#include "stm32f446xx.h"

typedef enum {
	MOTOR_SIDE_LEFT,
	MOTOR_SIDE_RIGHT
} MotorSide;

typedef enum {
	MOTOR_DIR_FWD,
	MOTOR_DIR_BCK,
	MOTOR_DIR_STOP
} MotorDir;

typedef enum {
	CAR_STATE_FORWARD,
	CAR_STATE_REVERSE,
	CAR_STATE_PIVOTLEFT,
	CAR_STATE_PIVOTRIGHT,
	CAR_STATE_STOPPED
} CarState;

void CarSetState(CarState state);
CarState CarGetState(void);
void CarCommand(uint8_t comm);
void CarSetSpeed(uint8_t speed);
uint8_t CarGetSpeed(void);
void SetWheelDir(MotorSide side, MotorDir dir, int32_t speed);
void CarForward();
void CarStop();

#endif
