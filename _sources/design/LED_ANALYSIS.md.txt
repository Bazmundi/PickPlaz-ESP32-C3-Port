# LED Analysis Report

## Overview
This report analyzes the LED implementation in the pickplazfeederstm project. The system uses PWM-controlled LEDs through Timer 1 (TIM1) and a single GPIO-controlled LED.

## LED Components

### PWM-Controlled LEDs (LED0-LED3) for animations
- **Timer**: TIM1
- **PWM Channels**: 4 channels (CH1-CH4)
- **Pins**:
  - LED0: PA11 (TIM1_CH4)
  - LED1: PA10 (TIM1_CH3)
  - LED2: PA9 (TIM1_CH2)
  - LED3: PA8 (TIM1_CH1)
- **PWM Configuration**:
  - Period: 2048
  - Prescaler: 0
  - Mode: PWM1
  - Polarity: High

### GPIO-Controlled LED (LED4)
- **Pin**: PB11
- **Control Method**: Simple GPIO on/off
- **Usage**: Feed operation indicator

### Opto Sensor LED
- **Pin**: PA5
- **Control Method**: Always on during operation
- **Purpose**: Illuminates the opto sensor.

## LED Behavior Analysis

### LED0-LED3 PWM Behavior
The four PWM-controlled LEDs are managed by the `eval_led_pwm()` function in `application.cpp`, which creates distinct patterns based on the application state:

#### 1. Idle State with Opto Indexed
**Condition**: `app_state == APP_idle` and `opto_is_indexed == true`

**Behavior**:
- LED0: OFF (0)
- LED1: OFF (0)
- LED2: OFF (0)
- LED3: ON (2048) - Full brightness

**Purpose**: Indicates system is idle and opto sensor is detecting the indexed position.


```{figure} ../images/IndexIdle.gif
---
align: center
figclass: doc-figure
---
Index Idle LED simulation
```

#### 2. Idle State without Opto Indexed
**Condition**: `app_state == APP_idle` and `opto_is_indexed == false`

**Behavior**:
- LED0: OFF (0)
- LED1: Sine wave animation based on `sintab[t2 % 256] * 8`
- LED2: Sine wave animation based on `sintab[t1 % 256] * 8`
- LED3: OFF (0)
- Phase difference: 128 steps between LED1 and LED2

**Purpose**: Creates a wave-like animation to indicate idle state without indexing.

```{figure} ../images/Unindexed.gif
---
align: center
figclass: doc-figure
---
Unindexed LED simulation
```

### Understanding Idle States

The system distinguishes between two idle conditions based on the opto sensor state:

#### Idle with Indexing (opto_is_indexed = true)
- The system is idle AND the opto sensor is currently detecting the reference/index position
- `opto_is_indexed` variable is set to 1 when ADC reading from the opto sensor is above the threshold (2800-3200)
- Visual indication: Solid light on LED3 indicates "ready and positioned at known reference point"

#### Idle without Indexing (opto_is_indexed = false)
- The system is idle BUT the opto sensor is NOT detecting the reference position
- `opto_is_indexed` variable is set to 0 when ADC reading from the opto sensor is below the threshold
- Visual indication: Wave animation on LED1-LED2 indicates "ready but looking for index position"

#### Practical Significance
- **Indexed**: The feeder mechanism knows exactly where it is (at the known reference position)
- **Not Indexed**: The feeder mechanism doesn't know its position and might need a calibration sequence
- The different LED patterns help operators understand whether the system is simply waiting (solid LED) or needs attention/positioning (animated LEDs)

#### 3. Forward Movement States
**Condition**: `app_state` is APP_increment_forward1, APP_increment_forward2, or APP_free_forward

**Behavior**:
- LED0: `sintab[t1 % 256] * 8` (wave pattern)
- LED1: `sintab[t2 % 256] * 8` (wave pattern)
- LED2: `sintab[t3 % 256] * 8` (wave pattern)
- LED3: `sintab[t4 % 256] * 8` (wave pattern)
- Phase spacing: `sine_speed` (55 steps) between each LED

**Purpose**: Creates a forward-moving wave animation to indicate forward motor movement.

```{figure} ../images/Forward.gif
---
align: center
figclass: doc-figure
---
Forward LED simulation
```

#### 4. Backward Movement States
**Condition**: `app_state` is APP_increment_backward1, APP_increment_backward2, or APP_free_backward

**Behavior**:
- LED3: `sintab[t1 % 256] * 8` (wave pattern)
- LED2: `sintab[t2 % 256] * 8` (wave pattern)
- LED1: `sintab[t3 % 256] * 8` (wave pattern)
- LED0: `sintab[t4 % 256] * 8` (wave pattern)
- Phase spacing: `sine_speed` (55 steps) between each LED, but in reverse order

**Purpose**: Creates a backward-moving wave animation to indicate backward motor movement.

```{figure} ../images/Backward.gif
---
align: center
figclass: doc-figure
---
Backward LED simulation
```

#### 5. Other States (Default)
**Condition**: All other application states

**Behavior**:
- LED0: `sintab[t1 % 256] * 8` (wave pattern)
- LED1: `sintab[t2 % 256] * 8` (wave pattern)
- LED2: `sintab[t3 % 256] * 8` (wave pattern)
- LED3: `sintab[t4 % 256] * 8` (wave pattern)
- Phase spacing: 128 steps between each LED

**Purpose**: Creates a standard wave animation for undefined states.

```{figure} ../images/DefaultSine.gif
---
align: center
figclass: doc-figure
---
Default Sine LED simulation
```


### LED4 (GPIO-Controlled)
LED4 is controlled by the `eval_led_feed()` function:

**Behavior**:
- Turns ON for a duration of 500ms when `feed_signal_state` changes from `FEED_none`
- Gradually counts down and turns OFF
- Controlled via `gpio_write(LED4_GPIO_Port, LED4_Pin, counter)`

**Purpose**: Provides visual feedback during feed operations.

### Opto Sensor LED
The opto sensor LED is controlled in `app_init()`:

**Behavior**:
- Set to ON permanently with `gpio_SetPin(OPTO_LED_GPIO_Port, OPTO_LED_Pin)`

**Purpose**: Illuminates the opto sensor for proper operation.

## Current LED Configuration Summary

| LED | Type | Control Method | Pin | Primary Function |
|-----|------|----------------|-----|------------------|
| LED0 | PWM | Timer 1 CH4 | PA11 | System state indication |
| LED1 | PWM | Timer 1 CH3 | PA10 | System state indication |
| LED2 | PWM | Timer 1 CH2 | PA9 | System state indication |
| LED3 | PWM | Timer 1 CH1 | PA8 | System state indication |
| LED4 | GPIO | Counter | PB11 | Feed operation feedback |
| OPTO_LED | GPIO | Static ON | PA5 | Opto sensor illumination |

*Total: 5 status LEDs + 1 opto LED = 6 LEDs*

## Timing and Animation Details

### Sine Wave Table
- 256-step sine lookup table
- Values range from 0-256
- Multiplied by 8 for LED brightness (0-2048 range)
- Creates smooth brightness transitions

### Animation Speed
- `sine_speed = 55` (used for movement animations)
- `uwTick` system timer provides time base
- Each LED updates on every SysTick interrupt (1kHz)

## Current Behavior Mapping

| Application State | LED Pattern | Visual Indication |
|-------------------|-------------|-------------------|
| Idle (indexed) | LED3=ON, LED0=LED1=LED2=OFF | System ready at index position |
| Idle (not indexed) | LED1=sinusoidal, LED2=sinusoidal+phase, LED0=LED3=OFF | System ready, searching for index |
| Forward motion | LED0→LED3 sequential wave | Motor moving forward |
| Backward motion | LED3→LED0 sequential wave | Motor moving backward |
| Other states | All LEDs sinusoidal pattern | Other system activity |
| Feed operation | LED4 pulses for 500ms | Feed operation active |
