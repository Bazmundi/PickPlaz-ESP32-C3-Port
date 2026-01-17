# Port stages

## Stage 0
Scope + pinout alignment — confirm ESP32‑C3 Super Mini pin mapping, voltage levels, peripherals, and ensure the target board definition is correct in platformio.ini.
Design details: [Porting design](../design/porting_design.md).

## Stage 1
Build skeleton + boot — get a minimal ESP‑IDF app compiling, flashing, and logging; establish serial console and reset/boot flow.
Lessons learned: [Stage 1](stage1_lessons.md), [Stage 1 QEMU](stage1_qemu_lessons.md).

## Stage 2
HAL abstraction — define a platform layer (GPIO, timers, PWM, UART, SPI/I2C, ADC) with stubbed APIs so higher‑level logic can compile without STM32 dependencies.
Plan: [Stage 2 plan](stage2_plan.md).

## Stage 3
Core peripherals bring‑up — implement GPIO + timing first, then UART/I2C/SPI, then ADC (noting C3 limitations), then PWM (LEDC), each with a quick self‑test.
Plan: [Stage 3 plan](stage3_plan.md).

## Stage 4
Feature porting by subsystem — migrate motor control, sensor inputs, and any safety/limit logic one module at a time, validating behavior against STM32 output.
Plan: [Stage 4 plan](stage4_plan.md).

## Stage 5
Runtime robustness — add watchdog, brownout considerations, error handling, and logging; confirm power/performance constraints on C3.

## Stage 6
Integration + calibration — run end‑to‑end behavior with calibration steps and validate motion/sensing timing.

## Stage 7

Verification + polish — consolidate configuration, clean up build flags, and document pin maps and bring‑up notes.
