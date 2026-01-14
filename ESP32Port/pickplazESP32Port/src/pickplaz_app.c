#include "pickplaz_app.h"

#include <inttypes.h>

#include "board_pins.h"
#include "esp_log.h"
#include "hal.h"
#include "hal_config.h"

static const char *TAG = "pickplaz_app";

extern const uint8_t sintab[256];

enum {
    APP_PWM_STM32_MAX = 2048,
    APP_TICK_HZ = 1000,
    APP_SINE_LEN = 256,
    APP_SINE_SCALE = 8,
    APP_FEED_PULSE_MS = 500,
    APP_BUTTON_CNT_MAX = 20,
    APP_BUTTON_LONGPRESS = 400,
};

typedef enum {
    FEED_none,
    FEED_short,
    FEED_long
} feed_signal_t;

typedef enum {
    FEED_fsm_low,
    FEED_fsm_high
} feed_fsm_t;

typedef enum {
    MOTOR_init,
    MOTOR_idle,
    MOTOR_running_forward,
    MOTOR_running_backward,
    MOTOR_brake
} motor_state_t;

typedef enum {
    APP_init,
    APP_idle,
    APP_increment_forward1,
    APP_increment_backward1,
    APP_increment_forward2,
    APP_increment_backward2,
    APP_free_forward,
    APP_free_backward
} app_state_t;

typedef enum {
    BUTTON_none,
    BUTTON_short,
    BUTTON_long,
    BUTTON_hold
} button_event_t;

typedef struct {
    int pin;
    bool active_low;
    uint32_t cnt;
    uint32_t press;
} app_button_t;

enum {
    MOTOR_FORWARD_NORMAL = 2048,
    MOTOR_BACKWARD_NORMAL = -2048,
    MOTOR_FORWARD_FAST = 2048,
    MOTOR_BACKWARD_FAST = -2048,
    MOTOR_STOP = 0
};

enum {
    APP_PWM_LED0_CH = 0,
    APP_PWM_LED1_CH = 1,
    APP_PWM_LED2_CH = 2,
    APP_PWM_LED3_CH = 3,
    APP_PWM_MOTOR_IN1_CH = 4,
    APP_PWM_MOTOR_IN2_CH = 5,
};

static app_button_t button_forward = {
    .pin = BOARD_GPIO_BUTTON_FWD,
    .active_low = BOARD_BUTTON_ACTIVE_LOW,
    .cnt = 0,
    .press = 0,
};

static app_button_t button_backward = {
    .pin = BOARD_GPIO_BUTTON_REV,
    .active_low = BOARD_BUTTON_ACTIVE_LOW,
    .cnt = 0,
    .press = 0,
};

static uint32_t app_tick_ms;
static uint32_t opto_is_indexed;

static feed_fsm_t feed_state;
static feed_signal_t feed_signal_state;
static uint32_t feed_timer;
static uint32_t feed_led_counter;

static uint32_t app_forward_request;
static uint32_t app_backward_request;
static uint32_t app_forward_continuous_rq;
static uint32_t app_backward_continuous_rq;
static app_state_t app_state = APP_init;
static uint32_t app_timer;

static int32_t motor_target;
static motor_state_t motor_state = MOTOR_init;
static uint32_t motor_timer;
static uint32_t motor_brake_pwm;
static bool motor_brake_forward;
static uint32_t motor_last_pwm;
static bool motor_last_forward;

static uint32_t sine_speed = 55;

static bool app_pin_valid(int pin) {
    return pin != BOARD_GPIO_UNUSED;
}

static hal_gpio_pull_t app_pull_for_active_low(bool active_low) {
    return active_low ? HAL_GPIO_PULL_UP : HAL_GPIO_PULL_DOWN;
}

static uint32_t app_pwm_max(void) {
    if (HAL_PWM_DUTY_RES_BITS >= 31) {
        return 0xFFFFFFFFU;
    }
    return (1U << HAL_PWM_DUTY_RES_BITS) - 1U;
}

static uint32_t app_pwm_scale(uint32_t value) {
    uint32_t max_duty = app_pwm_max();
    if (value >= APP_PWM_STM32_MAX) {
        return max_duty;
    }
    return (value * max_duty) / APP_PWM_STM32_MAX;
}

static void app_set_led_duty(int channel, uint32_t stm32_value) {
    if (stm32_value > APP_PWM_STM32_MAX) {
        stm32_value = APP_PWM_STM32_MAX;
    }
    hal_pwm_set_duty(channel, app_pwm_scale(stm32_value));
}

static bool app_gpio_is_active(int pin, bool active_low) {
    if (!app_pin_valid(pin)) {
        return false;
    }
    hal_gpio_level_t level = hal_gpio_read(pin);
    return active_low ? (level == HAL_GPIO_LOW) : (level == HAL_GPIO_HIGH);
}

static button_event_t app_button_update(app_button_t *button) {
    bool pressed = app_gpio_is_active(button->pin, button->active_low);

    if (pressed) {
        if (button->cnt < APP_BUTTON_CNT_MAX) {
            button->cnt++;
        }
    } else if (button->cnt > 0) {
        button->cnt--;
    }

    bool debounced = button->cnt > (APP_BUTTON_CNT_MAX / 2);
    if (debounced) {
        button->press++;
        if (button->press > APP_BUTTON_LONGPRESS) {
            return BUTTON_hold;
        }
        return BUTTON_none;
    }

    if (button->press) {
        button_event_t ev = BUTTON_none;
        if (button->press > APP_BUTTON_LONGPRESS) {
            ev = BUTTON_long;
        } else {
            ev = BUTTON_short;
        }
        button->press = 0;
        return ev;
    }
    return BUTTON_none;
}

static void app_set_motor(uint32_t pwm, bool forward) {
    uint32_t duty = app_pwm_scale(pwm);
    if (forward) {
        hal_pwm_set_duty(APP_PWM_MOTOR_IN1_CH, 0);
        hal_pwm_set_duty(APP_PWM_MOTOR_IN2_CH, duty);
    } else {
        hal_pwm_set_duty(APP_PWM_MOTOR_IN1_CH, duty);
        hal_pwm_set_duty(APP_PWM_MOTOR_IN2_CH, 0);
    }
}

static void run_feed_fsm(void) {
    if (!app_pin_valid(HAL_FEED_PIN)) {
        return;
    }

    bool feed_pin_state = app_gpio_is_active(HAL_FEED_PIN, HAL_FEED_ACTIVE_LOW);
    switch (feed_state) {
    case FEED_fsm_low:
        if (feed_pin_state) {
            feed_timer = 0;
            feed_state = FEED_fsm_high;
        }
        break;
    case FEED_fsm_high:
    default:
        if (!feed_pin_state) {
            if (feed_timer > 10) {
                feed_signal_state = FEED_long;
            } else {
                feed_signal_state = FEED_short;
            }
            feed_state = FEED_fsm_low;
        }
        feed_timer++;
        break;
    }
}

static void run_app_fsm(void) {
    switch (app_state) {
    case APP_init:
        app_state = APP_idle;
        app_timer = 200;
        break;
    case APP_idle:
        motor_target = MOTOR_STOP;
        if (app_forward_request || feed_signal_state == FEED_short) {
            app_state = APP_increment_forward1;
            app_forward_request = 0;
            app_timer = 500;
            if (feed_signal_state == FEED_short) {
                feed_signal_state = FEED_none;
            }
        }
        if (app_backward_request || feed_signal_state == FEED_long) {
            app_state = APP_increment_backward1;
            app_backward_request = 0;
            app_timer = 500;
            if (feed_signal_state == FEED_long) {
                feed_signal_state = FEED_none;
            }
        }
        if (app_forward_continuous_rq) {
            app_state = APP_free_forward;
        }
        if (app_backward_continuous_rq) {
            app_state = APP_free_backward;
        }
        break;
    case APP_increment_forward1:
        motor_target = MOTOR_FORWARD_NORMAL;
        if (!opto_is_indexed) {
            app_state = APP_increment_forward2;
            app_timer = 1500;
        }
        if (app_timer) {
            app_timer--;
        } else {
            app_state = APP_idle;
        }
        break;
    case APP_increment_forward2:
        motor_target = MOTOR_FORWARD_NORMAL;
        if (opto_is_indexed) {
            app_state = APP_idle;
        }
        if (app_timer) {
            app_timer--;
        } else {
            app_state = APP_idle;
        }
        break;
    case APP_increment_backward1:
        motor_target = MOTOR_BACKWARD_NORMAL;
        if (!opto_is_indexed) {
            app_state = APP_increment_backward2;
            app_timer = 1500;
        }
        if (app_timer) {
            app_timer--;
        } else {
            app_state = APP_idle;
        }
        break;
    case APP_increment_backward2:
        motor_target = MOTOR_BACKWARD_NORMAL;
        if (opto_is_indexed) {
            app_state = APP_idle;
        }
        if (app_timer) {
            app_timer--;
        } else {
            app_state = APP_idle;
        }
        break;
    case APP_free_forward:
        motor_target = MOTOR_FORWARD_FAST;
        if (!app_forward_continuous_rq) {
            app_state = APP_increment_forward2;
            app_timer = 1500;
        }
        break;
    case APP_free_backward:
        motor_target = MOTOR_BACKWARD_FAST;
        if (!app_backward_continuous_rq) {
            app_state = APP_increment_backward2;
            app_timer = 1500;
        }
        break;
    default:
        app_state = APP_init;
        break;
    }
}

static void run_motor_fsm(void) {
    switch (motor_state) {
    case MOTOR_init:
        app_set_motor(0, true);
        motor_state = MOTOR_idle;
        break;
    case MOTOR_idle:
        app_set_motor(0, true);
        if (motor_target > 0) {
            motor_state = MOTOR_running_forward;
        } else if (motor_target < 0) {
            motor_state = MOTOR_running_backward;
        }
        break;
    case MOTOR_running_forward:
        if (motor_target == 0) {
            motor_state = MOTOR_brake;
            motor_timer = 8;
            motor_brake_pwm = motor_last_pwm;
            motor_brake_forward = !motor_last_forward;
            app_set_motor(motor_brake_pwm, motor_brake_forward);
        } else {
            motor_last_pwm = (uint32_t)motor_target;
            motor_last_forward = true;
            app_set_motor((uint32_t)motor_target, true);
        }
        break;
    case MOTOR_running_backward:
        if (motor_target == 0) {
            motor_state = MOTOR_brake;
            motor_timer = 8;
            motor_brake_pwm = motor_last_pwm;
            motor_brake_forward = !motor_last_forward;
            app_set_motor(motor_brake_pwm, motor_brake_forward);
        } else {
            motor_last_pwm = (uint32_t)(-motor_target);
            motor_last_forward = false;
            app_set_motor((uint32_t)(-motor_target), false);
        }
        break;
    case MOTOR_brake:
        if (motor_timer) {
            motor_timer--;
        } else {
            motor_state = MOTOR_idle;
        }
        if (motor_target != 0) {
            motor_state = MOTOR_idle;
        }
        break;
    default:
        motor_state = MOTOR_init;
        break;
    }
}

static void eval_led_pwm(void) {
    uint32_t led0 = 0;
    uint32_t led1 = 0;
    uint32_t led2 = 0;
    uint32_t led3 = 0;

    if (app_state == APP_idle) {
        if (opto_is_indexed) {
            led3 = APP_PWM_STM32_MAX;
        } else {
            uint32_t t1 = app_tick_ms;
            uint32_t t2 = t1 + 128;
            led1 = sintab[t1 % APP_SINE_LEN] * APP_SINE_SCALE;
            led2 = sintab[t2 % APP_SINE_LEN] * APP_SINE_SCALE;
        }
    } else if (app_state == APP_increment_forward1 ||
               app_state == APP_increment_forward2 ||
               app_state == APP_free_forward) {
        uint32_t t1 = app_tick_ms;
        uint32_t t2 = t1 + sine_speed;
        uint32_t t3 = t2 + sine_speed;
        uint32_t t4 = t3 + sine_speed;
        led0 = sintab[t1 % APP_SINE_LEN] * APP_SINE_SCALE;
        led1 = sintab[t2 % APP_SINE_LEN] * APP_SINE_SCALE;
        led2 = sintab[t3 % APP_SINE_LEN] * APP_SINE_SCALE;
        led3 = sintab[t4 % APP_SINE_LEN] * APP_SINE_SCALE;
    } else if (app_state == APP_increment_backward1 ||
               app_state == APP_increment_backward2 ||
               app_state == APP_free_backward) {
        uint32_t t4 = app_tick_ms;
        uint32_t t3 = t4 + sine_speed;
        uint32_t t2 = t3 + sine_speed;
        uint32_t t1 = t2 + sine_speed;
        led0 = sintab[t1 % APP_SINE_LEN] * APP_SINE_SCALE;
        led1 = sintab[t2 % APP_SINE_LEN] * APP_SINE_SCALE;
        led2 = sintab[t3 % APP_SINE_LEN] * APP_SINE_SCALE;
        led3 = sintab[t4 % APP_SINE_LEN] * APP_SINE_SCALE;
    } else {
        uint32_t t1 = app_tick_ms;
        uint32_t t2 = t1 + 128;
        uint32_t t3 = t2 + 128;
        uint32_t t4 = t3 + 128;
        led0 = sintab[t1 % APP_SINE_LEN] * APP_SINE_SCALE;
        led1 = sintab[t2 % APP_SINE_LEN] * APP_SINE_SCALE;
        led2 = sintab[t3 % APP_SINE_LEN] * APP_SINE_SCALE;
        led3 = sintab[t4 % APP_SINE_LEN] * APP_SINE_SCALE;
    }

    if (app_pin_valid(BOARD_GPIO_LED0)) {
        app_set_led_duty(APP_PWM_LED0_CH, led0);
    }
    if (app_pin_valid(BOARD_GPIO_LED1)) {
        app_set_led_duty(APP_PWM_LED1_CH, led1);
    }
    if (app_pin_valid(BOARD_GPIO_LED2)) {
        app_set_led_duty(APP_PWM_LED2_CH, led2);
    }
    if (app_pin_valid(BOARD_GPIO_LED3)) {
        app_set_led_duty(APP_PWM_LED3_CH, led3);
    }
}

static void eval_led_feed(void) {
    if (feed_signal_state != FEED_none) {
        feed_led_counter = APP_FEED_PULSE_MS;
    }
    if (feed_led_counter) {
        feed_led_counter--;
    }
    if (app_pin_valid(BOARD_GPIO_LED4)) {
        hal_gpio_write(BOARD_GPIO_LED4,
                       feed_led_counter ? HAL_GPIO_HIGH : HAL_GPIO_LOW);
        return;
    }
    if (feed_led_counter) {
        app_set_led_duty(APP_PWM_LED3_CH, APP_PWM_STM32_MAX);
    }
}

static void app_update_opto(void) {
    if (app_pin_valid(HAL_OPTO_ADC_CHANNEL)) {
        int adc_value = hal_adc_read(HAL_OPTO_ADC_CHANNEL);
        if (adc_value >= 0) {
            if (opto_is_indexed) {
                opto_is_indexed = (uint32_t)adc_value > HAL_OPTO_ADC_LOW_THRESHOLD;
            } else {
                opto_is_indexed = (uint32_t)adc_value > HAL_OPTO_ADC_HIGH_THRESHOLD;
            }
        }
        return;
    }

    if (app_pin_valid(BOARD_GPIO_OPTO_INT)) {
        bool active = app_gpio_is_active(BOARD_GPIO_OPTO_INT, !HAL_OPTO_ACTIVE_HIGH);
        opto_is_indexed = active ? 1U : 0U;
    }
}

static void app_tick(void *user_data) {
    (void)user_data;
    app_tick_ms++;
#ifdef PICKPLAZ_APP_HEARTBEAT
    if ((app_tick_ms % APP_TICK_HZ) == 0) {
        ESP_LOGI(TAG, "Heartbeat tick=%" PRIu32 " state=%d motor=%ld opto=%" PRIu32,
                 app_tick_ms, app_state, (long)motor_target, opto_is_indexed);
    }
#endif

    switch (app_button_update(&button_forward)) {
    case BUTTON_short:
        app_forward_request = 1;
        break;
    case BUTTON_hold:
        app_forward_continuous_rq = 1;
        break;
    case BUTTON_none:
    case BUTTON_long:
    default:
        app_forward_continuous_rq = 0;
        break;
    }

    switch (app_button_update(&button_backward)) {
    case BUTTON_short:
        app_backward_request = 1;
        break;
    case BUTTON_hold:
        app_backward_continuous_rq = 1;
        break;
    case BUTTON_none:
    case BUTTON_long:
    default:
        app_backward_continuous_rq = 0;
        break;
    }

    app_update_opto();
    run_feed_fsm();
    run_app_fsm();
    run_motor_fsm();
    eval_led_pwm();
    eval_led_feed();
}

static void app_configure_pwm_outputs(void) {
    if (app_pin_valid(BOARD_GPIO_LED0)) {
        hal_pwm_init(APP_PWM_LED0_CH, BOARD_GPIO_LED0, HAL_PWM_LED_FREQ_HZ,
                     HAL_PWM_DUTY_RES_BITS);
    }
    if (app_pin_valid(BOARD_GPIO_LED1)) {
        hal_pwm_init(APP_PWM_LED1_CH, BOARD_GPIO_LED1, HAL_PWM_LED_FREQ_HZ,
                     HAL_PWM_DUTY_RES_BITS);
    }
    if (app_pin_valid(BOARD_GPIO_LED2)) {
        hal_pwm_init(APP_PWM_LED2_CH, BOARD_GPIO_LED2, HAL_PWM_LED_FREQ_HZ,
                     HAL_PWM_DUTY_RES_BITS);
    }
    if (app_pin_valid(BOARD_GPIO_LED3)) {
        hal_pwm_init(APP_PWM_LED3_CH, BOARD_GPIO_LED3, HAL_PWM_LED_FREQ_HZ,
                     HAL_PWM_DUTY_RES_BITS);
    }

    if (app_pin_valid(BOARD_GPIO_MOTOR_IN1)) {
        hal_pwm_init(APP_PWM_MOTOR_IN1_CH, BOARD_GPIO_MOTOR_IN1,
                     HAL_PWM_MOTOR_FREQ_HZ, HAL_PWM_DUTY_RES_BITS);
    }
    if (app_pin_valid(BOARD_GPIO_MOTOR_IN2)) {
        hal_pwm_init(APP_PWM_MOTOR_IN2_CH, BOARD_GPIO_MOTOR_IN2,
                     HAL_PWM_MOTOR_FREQ_HZ, HAL_PWM_DUTY_RES_BITS);
    }
}

static void app_configure_inputs(void) {
    if (app_pin_valid(button_forward.pin)) {
        hal_gpio_config_input(button_forward.pin,
                              app_pull_for_active_low(button_forward.active_low));
    }
    if (app_pin_valid(button_backward.pin)) {

        hal_gpio_config_input(button_backward.pin,
                              app_pull_for_active_low(button_backward.active_low));
    }
    if (app_pin_valid(HAL_FEED_PIN)) {
        hal_gpio_config_input(HAL_FEED_PIN, app_pull_for_active_low(HAL_FEED_ACTIVE_LOW));
    }
    if (app_pin_valid(BOARD_GPIO_OPTO_INT)) {
        hal_gpio_config_input(BOARD_GPIO_OPTO_INT,
                              app_pull_for_active_low(!HAL_OPTO_ACTIVE_HIGH));
    }
}

hal_status_t pickplaz_app_init(void) {
    ESP_LOGI(TAG, "PickPlaz app init (Stage 4)");

    app_configure_pwm_outputs();
    app_configure_inputs();
    if (app_pin_valid(BOARD_GPIO_LED4)) {
        hal_gpio_config_output(BOARD_GPIO_LED4, HAL_GPIO_LOW);
    }

    if (app_pin_valid(HAL_OPTO_ADC_CHANNEL)) {
        hal_adc_init();
    }

    feed_state = FEED_fsm_low;
    feed_signal_state = FEED_none;
    app_state = APP_init;
    motor_state = MOTOR_init;
    motor_target = MOTOR_STOP;
    motor_timer = 0;
    motor_last_pwm = 0;
    motor_last_forward = true;
    feed_timer = 0;
    feed_led_counter = 0;
    app_tick_ms = 0;

    return HAL_OK;
}

hal_status_t pickplaz_app_start(void) {
    return hal_tick_start(APP_TICK_HZ, app_tick, NULL);
}

void pickplaz_app_stop(void) {
    hal_tick_stop();
}
