#ifndef IR_H
#define IR_H

#include <stdint.h>

uint8_t IR_Decode(uint16_t* timestamps, uint16_t size);

#endif