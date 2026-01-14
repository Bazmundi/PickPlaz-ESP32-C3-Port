# Stage 3 Plan - Core Peripheral Bring-up (ESP32-C3)

Stage 3 implements real ESP-IDF drivers for the HAL interfaces created in
Stage 2. The goal is to bring up core peripherals in a controlled order with
small self-tests after each step.

## Goals
- Replace Stage 2 stubbed HAL calls with ESP-IDF implementations.
- Validate GPIO + timing first, then UART/I2C/SPI, then ADC, then PWM (LEDC).
- Keep each peripheral bring-up isolated and testable.
- Preserve Stage 1 console behavior and Stage 2 HAL API stability.

## Constraints
- Do not modify STM32 source files.
- Keep USB Serial/JTAG console for hardware builds.
- GPIO assignments and pin avoidance rules come from `PORTING_DESIGN.md`.

## Deliverables
1) **HAL GPIO implementation**
   - Configure outputs and inputs via `driver/gpio.h`.
   - Respect active-low button configuration.
2) **HAL timers + tick**
   - Implement `hal_timer_start/stop` using `esp_timer`.
   - Implement `hal_tick_start/stop` at 1 kHz.
3) **UART implementation**
   - Use UART0 (console is USB Serial/JTAG; avoid conflicts).
   - Provide basic read/write helpers.
4) **I2C and SPI skeletons**
   - Initialize bus with safe default pins (if not yet assigned).
   - Provide transfer stubs that return errors if bus not configured.
5) **ADC implementation (C3 limitations)**
   - Use `adc_oneshot` on ADC1 only.
   - Decide which GPIO can serve ADC1 input or defer to digital input.
6) **PWM (LEDC) implementation**
   - LEDC timer + channels for LEDs and motor pins.
   - Provide duty conversion and frequency setup.
7) **Self-test hooks**
   - A compile-time flag to enable tests per peripheral.

## Work breakdown
### 3.1 GPIO bring-up
- Implement:
  - `hal_gpio_config_output`, `hal_gpio_config_input`,
    `hal_gpio_write`, `hal_gpio_read`.
- Apply pull-ups for buttons:
  - FWD (GPIO20), REV (GPIO21), active-low.
- Self-test:
  - Toggle LED0..LED3 sequentially.
  - Log button states on change.

### 3.2 Timing + tick
- Implement:
  - `hal_timer_start/stop` using `esp_timer_create`.
  - `hal_tick_start/stop` with a 1 kHz periodic timer.
- Self-test:
  - Log tick count every 1000 ticks.
  - Verify no watchdog resets.

### 3.3 UART
- Implement:
  - `hal_uart_init`, `hal_uart_write`, `hal_uart_read` using `driver/uart.h`.
- Use UART0 for now and keep it optional (avoid breaking console).
- Self-test:
  - Write a short banner if UART init succeeds.

### 3.4 I2C and SPI
- Implement:
  - `hal_i2c_init`, `hal_i2c_read/write` using `driver/i2c.h`.
  - `hal_spi_init`, `hal_spi_transfer` using `driver/spi_master.h`.
- Keep pins configurable in a single header or compile-time defines.
- Self-test:
  - Log bus init success.
  - No external device required at this stage.

### 3.5 ADC (ESP32-C3 limits)
- Implement:
  - `hal_adc_init` and `hal_adc_read` using `adc_oneshot`.
- Use ADC1-capable GPIO only.
- Self-test:
  - Read sample value and log.
- Decision gate:
  - If opto is digital, keep ADC optional and return unsupported when unconfigured.

### 3.6 PWM (LEDC)
- Implement:
  - `hal_pwm_init`, `hal_pwm_set_duty` using `driver/ledc.h`.
- Use:
  - Motor IN1/IN2 (GPIO6/7) and LEDs (GPIO0/1/3/5).
- Self-test:
  - Sweep duty on LED channels.
  - Pulse motor pins at low duty with no motor attached.

## Integration steps
1) Replace HAL stubs in `src/hal.c` with ESP-IDF implementations.
2) Add a `HAL_SELFTEST` compile-time flag to run test routines.
3) Update `app_main` to run the staged self-tests when enabled.

## Acceptance criteria
- Project builds with real HAL implementations.
- GPIO toggles and button reads are correct (logs).
- Tick timer runs stably at 1 kHz.
- UART init does not interfere with USB Serial/JTAG console.
- ADC read returns sensible values (or cleanly reports unsupported).
- PWM output visible on assigned GPIOs.

## Risks and mitigations
- **Console conflicts (UART0)**: keep UART init optional and do not change
  console routing.
- **ADC pin constraints**: confirm which GPIO supports ADC1 on C3.
- **PWM frequency mismatch**: document chosen frequencies and adjust later.
- **Power/reset loops**: keep self-tests lightweight and optional.

## Open questions
- Confirm whether opto input is analog or digital on ESP32-C3.
- Decide I2C/SPI pin assignments (if not already in design docs).
- Determine if FEED input is used and its GPIO.

## Test notes
- For QEMU, use `scripts/run_qemu_with_monitor.sh` to avoid socket ordering.
- For hardware, keep monitor at 115200 and avoid DTR/RTS toggles.
