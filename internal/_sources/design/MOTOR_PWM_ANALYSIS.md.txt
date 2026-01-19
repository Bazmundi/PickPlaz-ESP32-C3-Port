# Motor PWM Analysis Report

## Overview
This report analyzes the PWM implementation for motor control in the pickplazfeederstm project. The system uses Timer 3 (TIM3) to generate PWM signals for controlling a DC motor with bidirectional movement capability.

## Motor PWM Components

### Timer Configuration
- **Timer**: TIM3
- **PWM Channels**: 2 channels (CH1 and CH2)
- **Pins**:
  - MOT_POS: PA6 (TIM3_CH1) - Positive pole
  - MOT_NEG: PA7 (TIM3_CH2) - Negative pole
- **PWM Configuration**:
  - Period: 2048
  - Prescaler: 0
  - Mode: PWM1
  - Polarity: High
  - Auto-reload preload: Enabled

### Motor Control Mechanism
The system implements an H-bridge motor control scheme:
- **Forward Movement**: CH1 = 0, CH2 = PWM value
- **Backward Movement**: CH1 = PWM value, CH2 = 0
- **Stop**: CH1 = 0, CH2 = 0
- **Brake**: CH1 = CH2 (both set to same value temporarily)

## PWM Implementation Details

### Hardware Setup
1. **Timer Initialization**: TIM3 configured with 2048 period and 0 prescaler
2. **Channel Configuration**: Both channels configured in PWM1 mode
3. **Pin Mapping**: PA6 and PA7 configured as alternate function for TIM3
4. **Startup**: PWM channels started in `app_init()` function

### Software Control
The motor control is implemented through multiple layers:

1. **Application State Machine**: Sets `motor_target` values based on system state
2. **Motor State Machine**: Translates `motor_target` into actual PWM signals
3. **Direct PWM Control**: `set_motor()` function directly controls CCR registers

### Motor Control Functions

#### set_motor() Function
```c
static void set_motor(uint32_t pwm, uint32_t direction) {
    //timer 3
    //ch1 is positive pole
    //ch2 is negative pole
    if (direction) {
        htim3.Instance->CCR1 = 0;
        htim3.Instance->CCR2 = pwm;
    } else {
        htim3.Instance->CCR1 = pwm;
        htim3.Instance->CCR2 = 0;
    }
}
```

**Parameters**:
- `pwm`: Value from 0 to 2048 representing motor speed
- `direction`: 1 for forward, 0 for backward

### Motor State Machine
The motor uses a 5-state FSM to control operation and implement braking:

1. **MOTOR_init**: Initial state, sets motor to stop
2. **MOTOR_idle**: Motor stopped, waiting for command
3. **Motor_running_forward**: Motor running forward at requested speed
4. **Motor_running_backward**: Motor running backward at requested speed
5. **Motor_break**: Active braking by shorting motor terminals

## Motor Behavior Analysis

### Speed Control
The PWM duty cycle directly controls motor speed:
- **0 (0%)**: Motor stopped
- **1024 (50%)**: Half speed
- **2048 (100%)**: Full speed

The system uses two speed levels:
- **Normal Speed**: 2048 (MOTOR_FORWARD_NORMAL/MOTOR_BACKWARD_NORMAL)
- **Fast Speed**: 2048 (MOTOR_FORWARD_FAST/MOTOR_BACKWARD_FAST)

### Direction Control
Direction is controlled by activating one PWM channel while keeping the other at 0:
- **Forward**: CH2 active (PA7), CH1 inactive (PA6)
- **Backward**: CH1 active (PA6), CH2 inactive (PA7)

### Braking Mechanism
When stopping, the system implements active braking:
1. Swaps CCR values temporarily (CH1=CH2, CH2=CH1)
2. Maintains this state for 8 timer cycles
3. This shorts the motor terminals, creating braking torque

## Application Integration

### Motor Target Values
The application uses specific constants for motor control:
- MOTOR_FORWARD_NORMAL = 2048
- MOTOR_BACKWARD_NORMAL = -2048
- MOTOR_FORWARD_FAST = 2048
- MOTOR_BACKWARD_FAST = -2048
- MOTOR_STOP = 0

### State Transitions
The motor responds to application state changes:
1. **Incremental movements**: Precise positioning control using opto sensor feedback
2. **Continuous movements**: Manual control with holding buttons
3. **Feed operations**: Automatic responses to feed signals

### Control Timing
- Motor updates occur in `app_systick()` which runs at 1kHz frequency
- Braking duration is fixed at 8 timer cycles
- State transitions are immediate for start/stop commands

## Timing Characteristics

### PWM Frequency
With TIM3 running at system clock speed and:
- Prescaler: 0
- Period: 2048
- Assuming 48MHz system clock: ~23.4kHz PWM frequency

### Response Time
- Motor commands processed every 1ms (SysTick interval)
- Braking duration: ~0.34ms (8 cycles at 23.4kHz)
- State transitions: Immediate

## Summary of Motor Functions

| Component | Type | Control | Primary Function |
|-----------|------|---------|------------------|
| TIM3_CH1 | PWM | PA6 | Positive motor pole |
| TIM3_CH2 | PWM | PA7 | Negative motor pole |
| Motor FSM | State Machine | Software | Motor state management |
| set_motor() | Function | Direct CCR | Low-level PWM control |

## Behavior Mapping

| Application State | Motor Target | Direction | Speed | Behavior |
|-------------------|--------------|-----------|-------|----------|
| APP_idle | MOTOR_STOP (0) | None | 0% | Motor stopped |
| APP_increment_forward1/2 | MOTOR_FORWARD_NORMAL (2048) | Forward | 100% | Precise forward movement |
| APP_increment_backward1/2 | MOTOR_BACKWARD_NORMAL (-2048) | Backward | 100% | Precise backward movement |
| APP_free_forward | MOTOR_FORWARD_FAST (2048) | Forward | 100% | Continuous forward movement |
| APP_free_backward | MOTOR_BACKWARD_FAST (-2048) | Backward | 100% | Continuous backward movement |
| Stopping | MOTOR_STOP (0) | Brake | N/A | Active braking for 8 cycles |

## Key Features

1. **Bidirectional Control**: Full forward and backward movement control
2. **Active Braking**: Reduces stopping time and prevents coasting
3. **Precise Speed Control**: 11-bit resolution (0-2048) for fine speed adjustment
4. **H-Bridge Implementation**: Efficient motor driver using complementary PWM
5. **State Management**: Robust FSM for reliable motor operation
6. **Real-time Response**: 1kHz update rate for responsive control