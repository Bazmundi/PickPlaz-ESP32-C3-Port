# PickPlaz ESP32-C3 Port - Stage 0 (Scope + Pinout Alignment)

This document captures the initial scope for the ESP32-C3 Super Mini port,
along with a first-pass pinout alignment from the STM32F030 baseline.
It also records from/to rationale and architecture issues to keep later
stages grounded.

## Stage 0 outcomes
- Captured baseline signals and pin roles from the STM32 project metadata.
- Documented expected peripheral requirements for the ESP32-C3 target.
- Flagged ambiguities between sources that must be resolved before wiring.
- Noted board configuration checks for PlatformIO.
- Confirmed a 5-LED layout for the ESP32-C3 port (LED0-LED4 supported).

## Existing analysis references
These documents remain the source of truth for behavior and timing details:
- `LED_ANALYSIS.md` (LED patterns, PWM use, idle/motion indications)
- `MOTOR_PWM_ANALYSIS.md` (motor PWM behavior, braking, timing)

## External references
- [ESP32-C3 hardware reference (ESP-IDF)](https://docs.espressif.com/projects/esp-idf/en/v5.5.2/esp32c3/hw-reference/index.html)
- [ESP32-C3 Super Mini overview (ESPBoards)](https://www.espboards.dev/esp32/esp32-c3-super-mini/)

## From/To rationale (high-level)
- From: STM32F030C8T6 uses STM32 HAL + direct timer PWM; single SysTick loop
  at 1 kHz drives application state, PWM updates, and sensor sampling.
- To: ESP32-C3 (ESP-IDF) favors driver APIs (LEDC for PWM, ADC driver, GPIO)
  and timer callbacks (esp_timer or FreeRTOS timers) to reproduce the 1 kHz
  behavior with deterministic timing.

Reasoning:
- ESP-IDF LEDC offers multiple high-resolution PWM channels suitable for LEDs
  and motor H-bridge outputs.
- ESP-IDF ADC drivers provide calibration support, but have different scaling
  and noise behavior than STM32 ADC; thresholds must be revalidated.
- A dedicated timer callback (1 kHz) best matches the existing SysTick model
  without rewriting the state machine.

## Architecture issues between from/to approaches
- **Timers/PWM**: STM32 TIMx PWM uses a single timer counter for multiple
  channels; ESP32 LEDC channels are grouped by timer but configured per
  channel. Duty ranges and frequency must be explicitly chosen to match
  ~20-25 kHz behavior.
- **ADC**: STM32 ADC is synchronous and lightly abstracted; ESP32 ADC has
  per-channel attenuation, calibration (eFuse), and limited ADC-capable pins.
  Opto threshold values need recalibration.
- **Interrupt + timing model**: STM32 SysTick ISR updates state at 1 kHz.
  ESP32 should use `esp_timer` or a FreeRTOS software timer to keep logic
  deterministic; avoid long ISR work.
- **GPIO default states**: ESP32 pins can be strapping pins with boot-time
  constraints; some pins default to input-only or have pull-ups. Pin choices
  must respect boot behavior and board routing.
- **Voltage domains**: STM32 is typically 3.3 V tolerant; ESP32-C3 is 3.3 V.
  Any 5 V signals from the feeder board must be level-shifted.

## Baseline signal inventory (STM32)
Source: `pickplazfeederstm.ioc`, `LED_ANALYSIS.md`, `MOTOR_PWM_ANALYSIS.md`.

### LEDs
Behavior details are in `LED_ANALYSIS.md`.
- LED0: PA11 (TIM1_CH4, PWM)
- LED1: PA10 (TIM1_CH3, PWM)
- LED2: PA9 (TIM1_CH2, PWM)
- LED3: PA8 (TIM1_CH1, PWM)
- LED4: PB11 (GPIO output, feed indicator)
- OPTO_LED: PA5 (GPIO output, always on)

STM32 opto driver behavior:
- The opto LED is driven as a plain GPIO and is turned on once in `app_init()`
  via `gpio_SetPin(OPTO_LED_GPIO_Port, OPTO_LED_Pin)`.
- There is no PWM or runtime modulation; it stays high for normal operation.

LED behavior summary (STM32):
Individual behaviors:
- LED4 is independent: a 500 ms pulse whenever `feed_signal_state` is not
  `FEED_none`.
- OPTO_LED is static on (not part of LED animations).

Grouped behaviors (LED0-LED3):
- Idle + indexed: LED3 full brightness; LED0/LED1/LED2 off.
- Idle + not indexed: LED1 and LED2 sine wave animation (phase offset);
  LED0 and LED3 off.
- Forward motion: LED0->LED3 sequential sine wave pattern.
- Backward motion: LED3->LED0 sequential sine wave pattern (reverse order).
- Other states: all four LEDs follow sine wave animation (phase 128 steps).

Notes for the ESP32 LED layout:
- LED0-LED3 retain the STM32 animation patterns.
- LED4 is available (GPIO10) for the dedicated feed pulse indicator.

### Motor (H-bridge PWM)
Behavior details are in `MOTOR_PWM_ANALYSIS.md`.
Notes: Motor analysis and .ioc labels disagree on polarity naming.

- .ioc labels:
  - MOT_NEG: PA6 (TIM3_CH1)
  - MOT_POS: PA7 (TIM3_CH2)
- Motor analysis text:
  - MOT_POS: PA6 (TIM3_CH1)
  - MOT_NEG: PA7 (TIM3_CH2)

Action: Confirm actual wiring or PCB schematic before assigning ESP32 pins.

### Inputs
- FEED: PA3 (GPIO input)
- OPTO_INTERRUPT: PA4 (ADC input)
- SW_FORWARD: PB2 (GPIO input)
- SW_BACKWARD: PB10 (GPIO input)

### Test points (optional in port)
- TP1: PB3 (GPIO output)
- TP2: PB4 (GPIO output)
- TP3: PB5 (GPIO output)
- TP4: PB6 (GPIO output)
- TP5: PB7 (GPIO output)

## ESP32-C3 Super Mini pinout alignment (confirmed)
This mapping is based on the provided ESP32-C3 Super Mini pinout list.

| Signal | STM32 Pin | ESP32-C3 Pin | Notes |
| --- | --- | --- | --- |
| IN1 (DRV8833) | PA6/PA7 | GPIO6 | PWM |
| IN2 (DRV8833) | PA7/PA6 | GPIO7 | PWM |
| Button FWD | PB2 | GPIO20 | Input, pull-up, active-low |
| Button REV | PB10 | GPIO21 | Input, pull-up, active-low |
| Opto interrupt | PA4 | GPIO4 | Input, configurable polarity |
| LED0 | PA11 | GPIO0 | Output |
| LED1 | PA10 | GPIO1 | Output |
| LED2 | PA9 | GPIO3 | Output |
| LED3 | PA8 | GPIO5 | Output |
| LED4 | PB11 | GPIO10 | Output (feed indicator) |

Notes:
- Avoid GPIO2, GPIO8, GPIO9 (strapping pins) and GPIO18/19 (USB).
- Motor polarity (forward/backward) should be verified in software during bring-up.
- LED4 is kept on ESP32-C3 (GPIO10) to preserve the STM32 feed indicator.
- OPTO_LED is hardware-powered from 3V3 (no GPIO).
- FEED is not assigned in this mapping; confirm if it is handled in hardware
  or needs a GPIO.

Update (Stage 4 hardware/QEMU split):
- Hardware mapping uses GPIO10 for LED4 (feed indicator).
- QEMU/PICSim mapping reassigns GPIO0/GPIO1 as FWD/REV buttons and disables
  LED0/LED1, keeping LED2/LED3 animations while leaving UART0 (GPIO20/21)
  available for logging.

## PlatformIO board configuration
Current `platformio.ini` uses:
- `platform = espressif32`
- `board = esp32-c3-devkitm-1`
- `framework = espidf`

Action: Confirm whether the Super Mini is best represented by
`esp32-c3-devkitm-1`, or if a dedicated board definition is required.
If a custom board is needed, create a `boards/` JSON entry before Stage 1.

## Stage 0 done / pending
Done:
- Captured STM32 signal roles and peripherals.
- Identified timing and ADC differences to account for in the port.
- Locked the initial ESP32-C3 GPIO assignments.

Pending (blockers for Stage 1):
- Resolve MOT_POS/MOT_NEG polarity mismatch.
- Choose the final PlatformIO board target or add a custom definition.
- Confirm how FEED is handled on the ESP32-C3 build.
