#include "stm32f446xx.h"
#include "gpio.h"
#include "pwm.h"
#include "car.h"
#include "tim.h"
#include "debug.h"
#include "dma.h"
#include "ir.h"
#include "config.h"

#include <stdio.h>
#include <stdbool.h>

// FRONT = L298N SIDE

#define BUF_SIZE 66

volatile uint32_t global_uptime_ms = 0;

// Buffer for timer input capture timestamps
uint16_t timestamps[BUF_SIZE];

volatile bool dma_complete = false;
volatile bool disable_dma = false;

volatile uint32_t encoder_left_last_capture = 0;
volatile uint32_t encoder_right_last_capture = 0;

volatile uint32_t encoder_left_period = STOPPED_PERIOD;
volatile uint32_t encoder_right_period = STOPPED_PERIOD;

volatile bool encoder_left_first_edge = true;
volatile bool encoder_right_first_edge = true;

volatile bool leftmotor = false;
volatile bool rightmotor = false;

static void MX_GPIO_Init(void) {
    // Enable IO clock for all active ports
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN | RCC_AHB1ENR_GPIOCEN;

    // Configure PC13 as On-board USER pushbutton with Pull-Up
    GPIO_SetMode(GPIOC, 13, G_GPIO_MODE_INPUT);
    GPIO_SetPull(GPIOC, 13, G_GPIO_PULL_UP);

    // Configure PA5 as On-board LED
    GPIO_SetMode(GPIOA, 5, G_GPIO_MODE_OUTPUT);

    // Configure PA6 for IR Receiver Input
    GPIO_SetMode(GPIOA, 6, G_GPIO_MODE_AF);
    GPIO_SetAF(GPIOA, 6, 2);

    // Configure PB6, PB7, PB8, PB9 for Motor PWM (TIM4)
    GPIO_SetMode(GPIOB, 6, G_GPIO_MODE_AF);
    GPIO_SetMode(GPIOB, 7, G_GPIO_MODE_AF);
    GPIO_SetMode(GPIOB, 8, G_GPIO_MODE_AF);
    GPIO_SetMode(GPIOB, 9, G_GPIO_MODE_AF);
    GPIO_SetAF(GPIOB, 6, 2);
    GPIO_SetAF(GPIOB, 7, 2);
    GPIO_SetAF(GPIOB, 8, 2);
    GPIO_SetAF(GPIOB, 9, 2);

    // Configure PA0 and PA1 for LM393 Input
    GPIO_SetMode(GPIOA, 0, G_GPIO_MODE_AF);
    GPIO_SetMode(GPIOA, 1, G_GPIO_MODE_AF);
    GPIO_SetAF(GPIOA, 0, 1);
    GPIO_SetAF(GPIOA, 1, 1);
}

static void MX_TIM4_Init(void) {
    // Enable the Motor PWM Timer (TIM4)
    RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;

    PWM_InitTypeDef pwm_init = { 
        .Prescaler = 0, 
        .Period = (TIM_GetClock(TIM4) / 20000) - 1, 
        .Mode = PWM_MODE_1 
    };

    PWM_Init(TIM4, 1, &pwm_init);
    PWM_Init(TIM4, 2, &pwm_init);
    PWM_Init(TIM4, 3, &pwm_init);
    PWM_Init(TIM4, 4, &pwm_init);
}

static void MX_TIM3_Init(void) {
    // IR Receiver Input Capture timer (1MHz counting)
    TIM_InitTypeDef tim3_init = { .Prescaler = (TIM_GetClock(TIM3) / 1000000) - 1, .Period = TIM_MAX_16BIT };
    TIM_Init(TIM3, &tim3_init);
    
    // Capture both edges to timestamp IR pulses
    TIM_InputCapture_Init(TIM3, 1, TIM_BOTH_EDGES);
    TIM_InputCapture_Init(TIM3, 2, TIM_BOTH_EDGES);

    TIM3->CCER &= ~TIM_CCER_CC2E;
    TIM3->CCMR1 &= ~TIM_CCMR1_CC2S;
    TIM3->CCMR1 |= TIM_CCMR1_CC2S_1;
    TIM3->CCER |= TIM_CCER_CC2E;
    
    // Enable timer interrupts for timeouts
    TIM3->DIER |= TIM_DIER_CC2IE;
    NVIC_EnableIRQ(TIM3_IRQn);
    TIM_Start(TIM3);
}

static void MX_TIM6_Init(void) {
    // Timeout timer for decoding instruction (10ms)
    TIM_InitTypeDef tim6_init = { .Prescaler = (TIM_GetClock(TIM6) / 1000000) - 1, .Period = 10000 };
    TIM_Init(TIM6, &tim6_init);
    TIM6->DIER |= TIM_DIER_UIE;
    NVIC_EnableIRQ(TIM6_DAC_IRQn);
}

static void MX_TIM7_Init(void) {
    // Timeout timer for repeat instructions (200ms)
    TIM_InitTypeDef tim7_init = { .Prescaler = (TIM_GetClock(TIM7) / 100000) - 1, .Period = 20000 };
    TIM_Init(TIM7, &tim7_init);
    TIM7->DIER |= TIM_DIER_UIE;
    NVIC_EnableIRQ(TIM7_IRQn);
}

static void MX_TIM2_Init(void) {
    // speed encoder
    TIM_InitTypeDef tim2_init = { .Prescaler = (TIM_GetClock(TIM2) / 1000000) - 1, .Period = TIM_MAX_32BIT };
    TIM_Init(TIM2, &tim2_init);

    TIM_InputCapture_Init(TIM2, 1, TIM_RISING_EDGE);
    TIM_InputCapture_Init(TIM2, 2, TIM_RISING_EDGE);

    TIM2->DIER |= TIM_DIER_CC1IE | TIM_DIER_CC2IE;
    NVIC_EnableIRQ(TIM2_IRQn);
    TIM_Start(TIM2);
}

static void MX_DMA_Init(void) {
    // Initialize DMA strictly for TIM3 CH1 transfers
    DMA_TIM3_CH1_Init(timestamps, BUF_SIZE);
}

static void PID_Update(void) {
    CarState state = CarGetState();
    
    if (state == CAR_STATE_STOPPED) {
        SetWheelDir(MOTOR_SIDE_LEFT, MOTOR_DIR_STOP, 0);
        SetWheelDir(MOTOR_SIDE_RIGHT, MOTOR_DIR_STOP, 0);
        return;
    } 
    if (!(state == CAR_STATE_FORWARD || state == CAR_STATE_REVERSE ||
          state == CAR_STATE_PIVOTLEFT || state == CAR_STATE_PIVOTRIGHT)) return;

    int32_t target_period = CarGetTargetPeriod();
    static float sum_err_left = 0, sum_err_right = 0;

    if (target_period == 99999999) {
        SetWheelDir(MOTOR_SIDE_LEFT, MOTOR_DIR_STOP, 0);
        SetWheelDir(MOTOR_SIDE_RIGHT, MOTOR_DIR_STOP, 0);
        sum_err_left = 0;
        sum_err_right = 0;
        return;
    }

    int32_t error_left = (int32_t)encoder_left_period - target_period; 
    int32_t error_right = (int32_t)encoder_right_period - target_period; 
    
    sum_err_left += (float)error_left;
    sum_err_right += (float)error_right;

    // Anti-Windup Clamping
    if (sum_err_left * PID_KI > PID_I_LIMIT_PCT) sum_err_left = PID_I_LIMIT_PCT / PID_KI;
    if (sum_err_left * PID_KI < -PID_I_LIMIT_PCT) sum_err_left = -PID_I_LIMIT_PCT / PID_KI;
    if (sum_err_right * PID_KI > PID_I_LIMIT_PCT) sum_err_right = PID_I_LIMIT_PCT / PID_KI;
    if (sum_err_right * PID_KI < -PID_I_LIMIT_PCT) sum_err_right = -PID_I_LIMIT_PCT / PID_KI;

    int32_t base_pwm = CarGetBasePWM();
    int32_t pwm_left = base_pwm;
    int32_t pwm_right = base_pwm;

    if (CarIsClosedLoop()) {
        pwm_left  += (int32_t)(PID_KP * (float)error_left) + (int32_t)(sum_err_left * PID_KI); 
        pwm_right += (int32_t)(PID_KP * (float)error_right) + (int32_t)(sum_err_right * PID_KI); 
    }

    switch(state) {
        case CAR_STATE_FORWARD:
            SetWheelDir(MOTOR_SIDE_LEFT, MOTOR_DIR_FWD, pwm_left);
            SetWheelDir(MOTOR_SIDE_RIGHT, MOTOR_DIR_FWD, pwm_right);
            break;
        case CAR_STATE_REVERSE:
            SetWheelDir(MOTOR_SIDE_LEFT, MOTOR_DIR_BCK, pwm_left);
            SetWheelDir(MOTOR_SIDE_RIGHT, MOTOR_DIR_BCK, pwm_right);
            break;
        case CAR_STATE_PIVOTLEFT:
            SetWheelDir(MOTOR_SIDE_LEFT, MOTOR_DIR_BCK, pwm_left);
            SetWheelDir(MOTOR_SIDE_RIGHT, MOTOR_DIR_FWD, pwm_right);
            break;
        case CAR_STATE_PIVOTRIGHT:
            SetWheelDir(MOTOR_SIDE_LEFT, MOTOR_DIR_FWD, pwm_left);
            SetWheelDir(MOTOR_SIDE_RIGHT, MOTOR_DIR_BCK, pwm_right);
            break;
    }
}


int main(void) {
    SysTick_Config(16000000 / 1000);
    // Initial hardware clock/debug setup
    Debug_Init();
    
    // Core Peripheral Initialization
    MX_GPIO_Init();
    MX_TIM4_Init();
    MX_TIM3_Init();
    MX_DMA_Init();
    MX_TIM6_Init();
    MX_TIM7_Init();
    MX_TIM2_Init();

    CarSetRPM(40);

    while(1) {
        if (dma_complete) {
            uint8_t comm = IR_Decode(timestamps, BUF_SIZE);
            CarCommand(comm);

            if (comm != 0xFF) GPIO_Write(GPIOA, 5, 1);

            DMA1_Stream4->NDTR = BUF_SIZE;
            DMA1_Stream4->M0AR = (uint32_t)timestamps;
            DMA1_Stream4->CR |= DMA_SxCR_EN;
            dma_complete = false;
        }
        //button
    	if (!(GPIOC->IDR & GPIO_IDR_ID13)) { // Pressed
           //GPIO_Write(GPIOA, 5, 0); 
    	} else { // Un-pressed
           //GPIO_Write(GPIOA, 5, 1); 
    	}
        if (leftmotor) {
            //printf("left motor: %lu\r\n", encoder_left_period);
            leftmotor = false;
        }
        if (rightmotor) {
            printf("right motor: %lu\r\n", 3000000/encoder_right_period);
            rightmotor = false;
        }
    }
}

void DMA1_Stream4_IRQHandler(void) {
    if (DMA1->HISR & DMA_HISR_TCIF4) {
        DMA1->HIFCR |= DMA_HIFCR_CTCIF4;
        dma_complete = true;
    }
}

void TIM3_IRQHandler(void) {
    if (TIM3->SR & TIM_SR_CC2IF) {
        TIM3->SR &= ~TIM_SR_CC2IF;
        TIM6->CNT = 0;
        TIM_Start(TIM6);
        TIM7->CNT = 0;
        TIM_Start(TIM7);
    }
}

void TIM6_DAC_IRQHandler(void) {
    if (TIM6->SR & TIM_SR_UIF) {
        TIM_Stop(TIM6);
        TIM6->SR &= ~TIM_SR_UIF;

        // clear buffer
        for (int i=0; i<BUF_SIZE; i++) {
            timestamps[i] = 0;
        }

        // reset DMA
        // Disable stream before config
        DMA1_Stream4->CR &= ~DMA_SxCR_EN;
        while (DMA1_Stream4->CR & DMA_SxCR_EN);

        DMA1_Stream4->NDTR = BUF_SIZE;
        DMA1_Stream4->M0AR = (uint32_t)timestamps;
        // Clear ALL flags for stream 4 before enabling
        DMA1->HIFCR = DMA_HIFCR_CTCIF4 | DMA_HIFCR_CHTIF4 | 
                        DMA_HIFCR_CTEIF4 | DMA_HIFCR_CDMEIF4 | DMA_HIFCR_CFEIF4;

        // Enable
        DMA1_Stream4->CR |= DMA_SxCR_EN;

    }
}

void TIM7_IRQHandler(void) {
    if (TIM7->SR & TIM_SR_UIF) {
        TIM_Stop(TIM7);
        TIM7->SR &= ~TIM_SR_UIF;
        CarSetState(CAR_STATE_STOPPED);
    }
}

void TIM2_IRQHandler(void) {
    if (TIM2->SR & TIM_SR_CC1IF) {
        TIM2->SR &= ~TIM_SR_CC1IF;
        if (encoder_right_first_edge) {
            encoder_right_last_capture = TIM2->CCR1;
            encoder_right_first_edge = false;
        } else {
            uint32_t raw_period = TIM2->CCR1 - encoder_right_last_capture;
            if (raw_period > (encoder_right_period / 2)) {
                encoder_right_period = raw_period;
                encoder_right_last_capture = TIM2->CCR1;
                rightmotor = true;
            }
        }
    }
    if (TIM2->SR & TIM_SR_CC2IF) {
        TIM2->SR &= ~TIM_SR_CC2IF;
        if (encoder_left_first_edge) {
            encoder_left_last_capture = TIM2->CCR2;
            encoder_left_first_edge = false;
        } else {
            uint32_t raw_period = TIM2->CCR2 - encoder_left_last_capture;
            if (raw_period > (encoder_left_period / 2)) {
                encoder_left_period = raw_period;
                encoder_left_last_capture = TIM2->CCR2;
                leftmotor = true;
            }
        }
    }
}

void SysTick_Handler(void) {
    global_uptime_ms++;

    if (global_uptime_ms % PID_INTERVAL_MS == 0) PID_Update();

    if ((TIM2->CNT - encoder_left_last_capture) > WATCHDOG_TIMEOUT_US) {
        encoder_left_period = CarGetTargetPeriod();
        encoder_left_first_edge = true;
    }
    if ((TIM2->CNT - encoder_right_last_capture) > WATCHDOG_TIMEOUT_US) {
        encoder_right_period = CarGetTargetPeriod();
        encoder_right_first_edge = true;
    }
}