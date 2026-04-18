# Bare-Metal STM32 RC Car

A high-performance, bare-metal C robotics platform built on the STM32F446RE Nucleo board. This project avoids the heavy STM32 HAL (Hardware Abstraction Layer) and Arduino bloat, directly manipulating memory-mapped registers via custom, strongly-typed C abstractions to achieve zero-latency DMA transfers and pure hardware-timer execution.

<!-- [Video demonstration to be added] -->

<p align="center">
  <img src="https://github.com/user-attachments/assets/c89e3380-e714-4fcd-8ee5-36035e8eff6d" width="45%" alt="RC Car Top View" />
  <img src="https://github.com/user-attachments/assets/96b30f0f-e2c7-4770-95cd-4ee70ba23e2e" width="45%" alt="RC Car Side View" />
</p>

## 🚀 Key Features

* **Closed-Loop PI Speed Control:**
  * RPM-targeted Proportional-Integral (PI) controller with Feedforward estimation, running at 100Hz from `SysTick_Handler`.
  * Independent per-wheel error tracking via dual LM393 optical encoder sensors on `TIM2` Input Capture (20-slot discs).
  * Anti-Windup clamping (±15% PWM contribution) with automatic accumulator reset on state change.
  * State-aware PWM floor: 30% minimum for linear motion, 70% for pivot (to overcome scrubbing stiction).
* **Runtime Mode Toggle (IR Remote):**
  * `ST/REPT` button switches between **Closed-Loop** (PI active) and **Open-Loop** (raw feedforward only) control.
  * Diagnostic LED (`PA5`) provides at-a-glance mode identification via inverted activation logic.
* **Centralized Configuration (`config.h`):**
  * All PID gains, PWM thresholds, watchdog timeouts, and system parameters in a single header file for rapid tuning.
* **Zero-CPU-Overhead IR Decoding (NEC Protocol):**
  * Incoming IR pulses are independently captured by the `TIM3` Hardware Input Capture peripheral (1MHz tick resolution).
  * Capture timestamps are seamlessly streamed into memory entirely over `DMA1 Stream 4`, demanding exactly 0% CPU cycles until a full 66-edge packet is complete.
* **Hardware-Level Watchdogs & Garbage Collection:**
  * Uses dedicated hardware timers (`TIM6` and `TIM7`) operating as independent watchdogs. They automatically identify and flush corrupted IR repeat-codes and autonomously halt the motors upon remote connection loss.
* **Silent PWM Motor Control:**
  * `TIM4` generates a 20kHz symmetric PWM signal across 4 channels, operating exactly on the boundary of human hearing to ensure the structural TT-motors run in absolute silence.
* **Robust Analog Architecture:**
  * Driven by dual DRV8833 H-Bridges in parallel.
  * Implements professional-grade physical routing, fully isolating the 3.3V Logic circuits from the 5V High-Current trace noise via a disciplined "Star Ground" chassis topology.
* **Professional Embedded C Patterns:**
  * Bypasses the monolithic STM32 HAL while still mirroring professional MISRA-C/HAL initialization structures (e.g., `TIM_InitTypeDef` configurations and strict `typedef enum` parameter enforcement) to guarantee absolute compile-time type safety.

## 🛠️ Hardware Requirements

*   **Microcontroller:** STM32F446RE (Nucleo-64)
*   **Motor Driver:** 2x DRV8833 Breakout Boards (Bridged logically for 4-wheel drive)
*   **Actuators:** 4x Standard TT Motors (Yellow)
*   **Sensors:** NEC-compatible 38kHz IR Receiver Module, 2x LM393 Optical Slot Encoders (20-slot discs)
*   **Power:** True split-rail power topology (USB isolated to MCU logic, High-Current battery bypassed directly to DRV8833 `VM` pins).

## 📁 Source Code Architecture

*   `/Core/Inc` & `/Core/Src`: Contains all modular, register-level drivers (`pwm`, `tim`, `gpio`, `dma`, `ir`, `car`) and centralized configuration (`config.h`).
*   `main.c`: Coordinates the `MX_` static initialization sequence, PID control loop, encoder ISRs, and hardware watchdogs.

## ⚙️ Compilation & Flashing

This project is built using the standard GNU ARM Embedded Toolchain (`arm-none-eabi`).

```bash
# Build the binary
make clean && make

# Flash to the STM32 via ST-LINK
st-flash write build/ir_remote_car.bin 0x8000000
```
