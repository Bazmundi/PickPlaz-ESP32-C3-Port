#include "hal.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <string.h>

static const char *TAG = "hal";

#define HAL_GPIO_MAX 40

static int8_t gpio_state[HAL_GPIO_MAX];
static bool gpio_state_init_done = false;

static void hal_gpio_state_init(void) {
    if (gpio_state_init_done) {
        return;
    }
    memset(gpio_state, -1, sizeof(gpio_state));
    gpio_state_init_done = true;
}

static bool hal_gpio_valid(int pin) {
    return (pin >= 0) && (pin < HAL_GPIO_MAX);
}

hal_status_t hal_init(void) {
    hal_gpio_state_init();
    ESP_LOGI(TAG, "HAL stub init (Stage 2)");
    return HAL_OK;
}

void hal_delay_ms(uint32_t ms) {
    if (ms == 0) {
        return;
    }
    vTaskDelay(pdMS_TO_TICKS(ms));
}

hal_status_t hal_gpio_config_output(int pin, hal_gpio_level_t initial_level) {
    if (!hal_gpio_valid(pin)) {
        return HAL_ERR_INVALID;
    }
    hal_gpio_state_init();
    gpio_state[pin] = (int8_t)initial_level;
    return HAL_OK;
}

hal_status_t hal_gpio_config_input(int pin, hal_gpio_pull_t pull) {
    if (!hal_gpio_valid(pin)) {
        return HAL_ERR_INVALID;
    }
    hal_gpio_state_init();
    if (gpio_state[pin] < 0) {
        gpio_state[pin] = HAL_GPIO_LOW;
    }
    (void)pull;
    return HAL_OK;
}

hal_status_t hal_gpio_write(int pin, hal_gpio_level_t level) {
    if (!hal_gpio_valid(pin)) {
        return HAL_ERR_INVALID;
    }
    hal_gpio_state_init();
    gpio_state[pin] = (int8_t)level;
    return HAL_OK;
}

hal_gpio_level_t hal_gpio_read(int pin) {
    if (!hal_gpio_valid(pin)) {
        return HAL_GPIO_LOW;
    }
    hal_gpio_state_init();
    if (gpio_state[pin] < 0) {
        return HAL_GPIO_LOW;
    }
    return (hal_gpio_level_t)gpio_state[pin];
}

hal_status_t hal_timer_start(int timer_id, uint32_t period_ms,
                             hal_timer_callback_t callback, void *user_data) {
    (void)timer_id;
    (void)period_ms;
    (void)callback;
    (void)user_data;
    return HAL_ERR_UNSUPPORTED;
}

hal_status_t hal_timer_stop(int timer_id) {
    (void)timer_id;
    return HAL_ERR_UNSUPPORTED;
}

hal_status_t hal_tick_start(uint32_t hz, hal_timer_callback_t callback, void *user_data) {
    (void)hz;
    (void)callback;
    (void)user_data;
    return HAL_ERR_UNSUPPORTED;
}

hal_status_t hal_tick_stop(void) {
    return HAL_ERR_UNSUPPORTED;
}

hal_status_t hal_pwm_init(int channel, int pin, uint32_t freq_hz,
                          uint32_t duty_resolution_bits) {
    (void)channel;
    (void)pin;
    (void)freq_hz;
    (void)duty_resolution_bits;
    return HAL_ERR_UNSUPPORTED;
}

hal_status_t hal_pwm_set_duty(int channel, uint32_t duty) {
    (void)channel;
    (void)duty;
    return HAL_ERR_UNSUPPORTED;
}

hal_status_t hal_uart_init(int uart_id, uint32_t baud_rate) {
    (void)uart_id;
    (void)baud_rate;
    return HAL_ERR_UNSUPPORTED;
}

int hal_uart_write(int uart_id, const uint8_t *data, size_t length) {
    (void)uart_id;
    (void)data;
    (void)length;
    return HAL_ERR_UNSUPPORTED;
}

int hal_uart_read(int uart_id, uint8_t *data, size_t length) {
    (void)uart_id;
    (void)data;
    (void)length;
    return HAL_ERR_UNSUPPORTED;
}

hal_status_t hal_spi_init(int bus_id, uint32_t clock_hz, uint8_t mode) {
    (void)bus_id;
    (void)clock_hz;
    (void)mode;
    return HAL_ERR_UNSUPPORTED;
}

int hal_spi_transfer(int bus_id, const uint8_t *tx, uint8_t *rx, size_t length) {
    (void)bus_id;
    (void)tx;
    (void)rx;
    (void)length;
    return HAL_ERR_UNSUPPORTED;
}

hal_status_t hal_i2c_init(int bus_id, uint32_t clock_hz) {
    (void)bus_id;
    (void)clock_hz;
    return HAL_ERR_UNSUPPORTED;
}

int hal_i2c_write(int bus_id, uint8_t addr, const uint8_t *data, size_t length) {
    (void)bus_id;
    (void)addr;
    (void)data;
    (void)length;
    return HAL_ERR_UNSUPPORTED;
}

int hal_i2c_read(int bus_id, uint8_t addr, uint8_t *data, size_t length) {
    (void)bus_id;
    (void)addr;
    (void)data;
    (void)length;
    return HAL_ERR_UNSUPPORTED;
}

hal_status_t hal_adc_init(void) {
    return HAL_ERR_UNSUPPORTED;
}

int hal_adc_read(int channel) {
    (void)channel;
    return HAL_ERR_UNSUPPORTED;
}
