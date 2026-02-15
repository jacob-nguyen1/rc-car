#ifndef CAR_H
#define CAR_H

#include <stdint.h>
#include <stdbool.h>
#include "stm32f4xx.h"

typedef enum {
	LEFT,
	RIGHT
} MotorSide;

typedef enum {
	FWD,
	BCK,
	STOP
} MotorDir;

void SetWheelDir(MotorSide side, MotorDir dir);
void CarForward();
void CarStop();

#endif
