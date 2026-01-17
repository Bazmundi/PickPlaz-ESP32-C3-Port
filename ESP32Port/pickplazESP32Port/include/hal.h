/*
 * PickPlaz ESP32-C3 Port
 * Copyright (c) 2026 Asterion Daedalus https://github.com/Bazmundi
 * SPDX-License-Identifier: MIT
 *
 * This file is part of PickPlaz ESP32-C3 Port and is licensed under the MIT License.
 * See the LICENSE file in the project root for full license text.
 */

#ifndef PICKPLAZ_HAL_H_
#define PICKPLAZ_HAL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum {
    HAL_OK = 0,
    HAL_ERR_UNSUPPORTED = -1,
    HAL_ERR_INVALID = -2
} hal_status_t;

typedef enum {
    HAL_GPIO_LOW = 0,
    HAL_GPIO_HIGH = 1
} hal_gpio_level_t;

typedef enum {
    HAL_GPIO_PULL_NONE = 0,
    HAL_GPIO_PULL_UP,
    HAL_GPIO_PULL_DOWN
} hal_gpio_pull_t;

typedef void (*hal_timer_callback_t)(void *user_data);

hal_status_t hal_init(void);
void hal_delay_ms(uint32_t ms);

hal_status_t hal_gpio_config_output(int pin, hal_gpio_level_t initial_level);
hal_status_t hal_gpio_config_input(int pin, hal_gpio_pull_t pull);
hal_status_t hal_gpio_write(int pin, hal_gpio_level_t level);
hal_gpio_level_t hal_gpio_read(int pin);

hal_status_t hal_timer_start(int timer_id, uint32_t period_ms,
                             hal_timer_callback_t callback, void *user_data);
hal_status_t hal_timer_stop(int timer_id);

hal_status_t hal_tick_start(uint32_t hz, hal_timer_callback_t callback, void *user_data);
hal_status_t hal_tick_stop(void);

hal_status_t hal_pwm_init(int channel, int pin, uint32_t freq_hz,
                          uint32_t duty_resolution_bits);
hal_status_t hal_pwm_set_duty(int channel, uint32_t duty);

hal_status_t hal_uart_init(int uart_id, uint32_t baud_rate);
int hal_uart_write(int uart_id, const uint8_t *data, size_t length);
int hal_uart_read(int uart_id, uint8_t *data, size_t length);

hal_status_t hal_spi_init(int bus_id, uint32_t clock_hz, uint8_t mode);
int hal_spi_transfer(int bus_id, const uint8_t *tx, uint8_t *rx, size_t length);

hal_status_t hal_i2c_init(int bus_id, uint32_t clock_hz);
int hal_i2c_write(int bus_id, uint8_t addr, const uint8_t *data, size_t length);
int hal_i2c_read(int bus_id, uint8_t addr, uint8_t *data, size_t length);

hal_status_t hal_adc_init(void);
int hal_adc_read(int channel);

void hal_selftest_run(void);

#ifdef __cplusplus
}
#endif

#endif
