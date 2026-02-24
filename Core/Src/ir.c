#include "ir.h"

uint8_t IR_Decode(uint16_t* timestamps, uint16_t size) {
    uint16_t pulses[size-1];
    int p = 0;
    for (int i = 1; i < size; i++) {
        uint16_t t1 = timestamps[i-1];
        uint16_t t2 = timestamps[i];
        
        uint16_t diff;
        if (t2 >= t1) {
            diff = t2 - t1;  // Normal case
        } else {
            diff = (65536 - t1) + t2;  // Wrapped
        }
        
        pulses[p++] = diff;
    } 


    int start = -1;  // Error value

    // find 9000 4500
    for (int i=1; i<size-1; i++) {
        if (pulses[i-1] > 8500 && pulses[i-1] < 9500 && pulses[i] > 4000 && pulses[i] < 5000) {
            start = i+1;
            break;
        }
    }

    if (start < 0 || start > 15) {
        return 0xFF;
    } 

    uint8_t comm = 0;
    for (int bit = 0; bit < 8; bit++) {
        int pulse_idx = start + 32 + (bit * 2) + 1;
        if (pulses[pulse_idx] > 1000) {
            comm |= (1 << bit);
        }
    }
    return comm;
}