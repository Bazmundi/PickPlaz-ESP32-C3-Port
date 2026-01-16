# Stage 4 Plan - Feature Porting by Subsystem (ESP32-C3)

Stage 4 ports actual application behavior from the STM32 firmware to the ESP32
by migrating subsystems one at a time. Each subsystem is validated against the
known STM32 behavior or documentation before moving on.

## Goals
- Port motor control, sensor inputs, and safety/limit logic in isolation.
- Avoid regressions by validating each subsystem independently.
- Keep HAL API stable while moving application logic across.

## Constraints
- Do not modify STM32 source files.
- Use the Stage 3 HAL for all hardware access.
- Maintain the ESP32 pin map in `PORTING_DESIGN.md`.

## Deliverables
1) **Motor control subsystem**
   - PWM drive for DRV8833 IN1/IN2.
   - Direction control and duty mapping aligned to STM32 behavior.
2) **Sensor input subsystem**
   - Buttons (FWD/REV) with active‑low logic.
   - Opto interrupt handling (polarity configurable).
3) **Safety/limit logic**
   - Any debounce, timeout, or edge‑case handling from STM32.
   - Enforce safe stop on invalid states.
4) **Validation harness**
   - Small test hooks to compare behavior across platforms.

## Work breakdown
### 4.1 Motor control port
- Identify motor drive logic and duty mapping in STM32 code.
- Map to HAL PWM calls:
  - `hal_pwm_init` for IN1/IN2.
  - `hal_pwm_set_duty` for speed updates.
- Validate:
  - Stop, forward, reverse transitions.
  - Duty ramp behavior (if present).
- Self‑test:
  - Low‑duty pulses with motor disconnected.

### 4.2 Button input + debounce
- Port button handling logic from STM32:
  - Press, release, long‑press, hold.
- Use `hal_gpio_read` with active‑low interpretation.
- Validate:
  - Timing and event generation match STM32 behavior.
  - Debounce counters align with STM32 tick assumptions.

### 4.3 Opto interrupt handling
- Confirm whether opto is digital interrupt or analog sampling.
- If digital:
  - Configure GPIO input and optional edge interrupt handler.
  - Provide a HAL wrapper to expose polarity configuration.
- If analog:
  - Use `hal_adc_read` with documented scaling.
- Validate:
  - Edge detection and polarity control match STM32 logic.

### 4.4 Safety/limit logic port
- Port limit/safety handling (timeouts, conflicting inputs, etc.).
- Ensure:
  - Motor stops on invalid inputs.
  - Buttons/inputs do not fight each other.
- Validate:
  - Known STM32 scenarios reproduced in logs.

### 4.5 STM32 behavior comparison
- Capture expected behaviors from existing STM32 logs or docs.
- Use ESP32 logs to compare:
  - Motor duty changes.
  - Button event transitions.
  - Opto event timing.

## Integration steps
1) Add subsystem wrappers on top of HAL in ESP32 app code.
2) Port subsystem logic incrementally (motor → buttons → opto → safety).
3) Add temporary logging to validate each subsystem.
4) Remove temporary logging once validated.

## Acceptance criteria
- Motor control works with correct direction and duty mapping.
- Button events match STM32 behavior (short/long/hold).
- Opto input handled correctly with configurable polarity.
- Safety logic halts motor on invalid or unsafe conditions.

## Risks and mitigations
- **Timing mismatch**: align tick rate and debounce timings with STM32.
- **PWM frequency mismatch**: document and adjust if motor behavior differs.
- **Sensor polarity**: confirm opto polarity to avoid inverted behavior.
- **Concurrency**: avoid long work in ISR; use task notifications if needed.

## Open questions
- Exact motor duty scaling from STM32 (0..2048 vs 0..1023).
- Opto input type (digital vs analog).
- Any additional limits or edge cases in STM32 logic not yet mapped.

## Test notes
- Prefer QEMU for logic verification; use hardware for timing validation.
- Use the Stage 3 self‑test to confirm base HAL before subsystem testing.
