#ifndef CONFIG_H
#define CONFIG_H

// --- PID Constants ---
#define PID_KP              0.0005f
#define PID_KI              0.000018f
#define PID_I_LIMIT_PCT     15.0f   // Anti-windup limit (+/- PWM %)
#define PID_INTERVAL_MS     10

// --- Encoder & Speed ---
#define STOPPED_PERIOD          99999999
#define ENCODER_TICKS_PER_REV   20
#define RPM_TO_PERIOD_CONST     3000000 // 60s / 20 slots * 1MHz
#define MAX_RPM_LINEAR          130
#define MAX_RPM_PIVOT           100     // Pivots are slower due to friction

// --- PWM Thresholds ---
#define PWM_FLOOR_LINEAR        30
#define PWM_FLOOR_PIVOT         70      // Overcome scrubbing friction
#define PWM_CEILING             100

// --- Safety & Hardware ---
#define WATCHDOG_TIMEOUT_US     200000

#endif
