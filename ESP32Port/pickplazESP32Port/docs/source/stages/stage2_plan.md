# Stage 2 Plan - HAL + Driver Scaffolding (ESP32-C3)

Stage 2 focuses on creating a minimal hardware abstraction layer (HAL) and
driver scaffolding so the STM32 application logic can compile and run on ESP32.
This stage does not port full behavior yet; it sets up the plumbing.

## Goals
- Define a stable HAL API that mirrors the STM32 application's needs.
- Implement ESP32-C3 GPIO, PWM (LEDC), and timing primitives.
- Establish a 1 kHz tick callback to match the STM32 SysTick model.
- Keep UART console on USB Serial/JTAG (Stage 1 outcome).

## Inputs and constraints
- **Pin map (confirmed)**: see `PORTING_DESIGN.md`.
- **LED4 omitted**: feed indication must reuse LED3 per `LED_ANALYSIS.md`.
- **OPTO_LED**: hardware-powered from 3V3 (no GPIO).
- **FEED**: still unresolved; must confirm if it's a GPIO input or handled in hardware.
- **No STM32 edits**: do not modify STM32 files.

## Deliverables
1) **Pin definition header**
   - `include/board_pins.h` with ESP32 GPIO assignments.
   - Centralizes any polarity (active-low buttons) and pull-ups.

2) **HAL API**
   - `include/hal.h` defining the minimum set of functions needed by the app.
   - Example functions (final list after audit):
     - `hal_init()`
     - `hal_gpio_write(pin, level)`
     - `hal_gpio_read(pin)`
     - `hal_pwm_init(...)`
     - `hal_pwm_set(channel, duty)`
     - `hal_adc_read(channel)` or `hal_opto_read()` (depending on sensor type)
     - `hal_tick_start(callback, hz)`
     - `hal_delay_ms(ms)`

3) **HAL implementation**
   - `src/hal.c` implementing the above using ESP-IDF:
     - GPIO driver for buttons and simple LEDs.
     - LEDC for PWM outputs (LED0-LED3 + DRV8833 IN1/IN2).
     - `esp_timer` for 1 kHz tick.
     - Optional: `adc_oneshot` for opto if analog; otherwise GPIO input.

4) **Minimal hardware self-test**
   - A tiny routine called from `app_main()` (guarded by a build flag) to:
     - Toggle LED0-LED3 sequentially.
     - Read buttons (FWD/REV) and log state.
     - Exercise motor PWM outputs at low duty (no motor connected).

## Work breakdown
### 2.1 Audit STM32 hardware usage
- Identify every hardware call used by the STM32 logic (GPIO, PWM, ADC, tick).
- Produce a short mapping list to drive the HAL API.
- Output: checklist of required functions and expected semantics.

### 2.2 Define the HAL interface
- Create `include/hal.h` with clear, minimal functions.
- Keep API plain C and explicit (avoid hidden global state).
- Ensure the interface is stable enough for Stage 3 porting.

### 2.3 Implement GPIO primitives
- Configure outputs for LED0-LED3 and optional test points.
- Configure inputs for:
  - `Button FWD` (GPIO20, pull-up, active-low)
  - `Button REV` (GPIO21, pull-up, active-low)
  - `Opto interrupt` (GPIO4, configurable polarity)
- Provide a helper for active-low reads.

### 2.4 Implement PWM via LEDC
- Define one LEDC timer for LEDs and one for motor (or reuse if frequency matches).
- Target PWM frequency ~20-25 kHz for motor, match LED smoothing needs.
- Provide duty range consistent with STM32 (0..2048) or convert internally.
- Map channels:
  - LED0: GPIO0
  - LED1: GPIO1
  - LED2: GPIO3
  - LED3: GPIO5
  - Motor IN1: GPIO6
  - Motor IN2: GPIO7

### 2.5 Implement 1 kHz tick
- Use `esp_timer` to call a `hal_tick_callback()` at 1 kHz.
- Keep the callback short; push work to the main loop if needed.

### 2.6 Opto input handling decision
- Confirm whether the opto is analog (ADC) or digital interrupt.
  - If analog: pick an ADC1-capable GPIO and configure `adc_oneshot`.
  - If digital: configure GPIO input and optional edge interrupt.
- Define a single HAL call so higher layers don't care.

### 2.7 Integrate into `app_main`
- Initialize HAL.
- Start the 1 kHz tick.
- Optional: run self-test if a `CONFIG_PICKPLAZ_SELFTEST` flag is set.

## Acceptance criteria
- Project builds cleanly with the new HAL code.
- USB Serial/JTAG console logs show HAL init completion.
- LEDs toggle in self-test (visual or scope).
- Buttons read correctly (logs show active-low changes).
- PWM output visible on GPIO6/GPIO7 (scope or motor disconnected).

## Risks and mitigations
- **USB console not active**: ensure `board_build.sdkconfig` is set and
  `CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG=y` in the build output.
- **PWM frequency mismatch**: document the chosen frequency; adjust once
  motor behavior is validated.
- **Opto signal type unknown**: keep HAL API abstract until confirmed.
- **Motor polarity mismatch**: verify by scope before connecting motor.

## Open questions (to resolve during Stage 2)
- Is FEED used in the ESP32 build? If yes, assign a GPIO.
- Is opto input analog or digital?
- Do we need any of TP1-TP5 on the ESP32 build?

## Test notes
- Always start monitor before reset to capture boot logs.
- If USB flaps, confirm device enumeration with `lsusb` and `ls /dev/ttyACM*`.
