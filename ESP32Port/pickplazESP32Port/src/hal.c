#include "hal.h"

#include "hal_config.h"

#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"

#include "driver/gpio.h"
#include "driver/i2c.h"
#include "driver/ledc.h"
#include "driver/spi_master.h"
#include "driver/uart.h"
#include "esp_adc/adc_oneshot.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "hal";

static bool hal_gpio_valid(int pin) {
    return (pin >= 0) && GPIO_IS_VALID_GPIO((gpio_num_t)pin);
}

static bool hal_gpio_output_valid(int pin) {
    return (pin >= 0) && GPIO_IS_VALID_OUTPUT_GPIO((gpio_num_t)pin);
}

hal_status_t hal_init(void) {
    ESP_LOGI(TAG, "HAL init (Stage 3)");
    return HAL_OK;
}

void hal_delay_ms(uint32_t ms) {
    if (ms == 0) {
        return;
    }
    vTaskDelay(pdMS_TO_TICKS(ms));
}

hal_status_t hal_gpio_config_output(int pin, hal_gpio_level_t initial_level) {
    if (!hal_gpio_output_valid(pin)) {
        return HAL_ERR_INVALID;
    }

    gpio_config_t cfg = {
        .pin_bit_mask = 1ULL << pin,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    if (gpio_config(&cfg) != ESP_OK) {
        return HAL_ERR_INVALID;
    }
    gpio_set_level((gpio_num_t)pin, (int)initial_level);
    return HAL_OK;
}

hal_status_t hal_gpio_config_input(int pin, hal_gpio_pull_t pull) {
    if (!hal_gpio_valid(pin)) {
        return HAL_ERR_INVALID;
    }

    gpio_config_t cfg = {
        .pin_bit_mask = 1ULL << pin,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = (pull == HAL_GPIO_PULL_UP) ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE,
        .pull_down_en = (pull == HAL_GPIO_PULL_DOWN) ? GPIO_PULLDOWN_ENABLE : GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    if (gpio_config(&cfg) != ESP_OK) {
        return HAL_ERR_INVALID;
    }
    return HAL_OK;
}

hal_status_t hal_gpio_write(int pin, hal_gpio_level_t level) {
    if (!hal_gpio_output_valid(pin)) {
        return HAL_ERR_INVALID;
    }
    gpio_set_level((gpio_num_t)pin, (int)level);
    return HAL_OK;
}

hal_gpio_level_t hal_gpio_read(int pin) {
    if (!hal_gpio_valid(pin)) {
        return HAL_GPIO_LOW;
    }
    return (hal_gpio_level_t)gpio_get_level((gpio_num_t)pin);
}

#define HAL_TIMER_MAX 4

typedef struct {
    esp_timer_handle_t handle;
    hal_timer_callback_t callback;
    void *user_data;
} hal_timer_entry_t;

static hal_timer_entry_t hal_timers[HAL_TIMER_MAX];

static void hal_timer_dispatch(void *arg) {
    hal_timer_entry_t *entry = (hal_timer_entry_t *)arg;
    if (entry->callback) {
        entry->callback(entry->user_data);
    }
}

hal_status_t hal_timer_start(int timer_id, uint32_t period_ms,
                             hal_timer_callback_t callback, void *user_data) {
    if (timer_id < 0 || timer_id >= HAL_TIMER_MAX || callback == NULL || period_ms == 0) {
        return HAL_ERR_INVALID;
    }

    hal_timer_entry_t *entry = &hal_timers[timer_id];
    if (entry->handle) {
        esp_timer_stop(entry->handle);
        esp_timer_delete(entry->handle);
        entry->handle = NULL;
    }

    esp_timer_create_args_t args = {
        .callback = &hal_timer_dispatch,
        .arg = entry,
        .name = "hal_timer"
    };
    if (esp_timer_create(&args, &entry->handle) != ESP_OK) {
        entry->handle = NULL;
        return HAL_ERR_INVALID;
    }

    entry->callback = callback;
    entry->user_data = user_data;

    uint64_t period_us = (uint64_t)period_ms * 1000ULL;
    if (esp_timer_start_periodic(entry->handle, period_us) != ESP_OK) {
        esp_timer_delete(entry->handle);
        entry->handle = NULL;
        return HAL_ERR_INVALID;
    }

    return HAL_OK;
}

hal_status_t hal_timer_stop(int timer_id) {
    if (timer_id < 0 || timer_id >= HAL_TIMER_MAX) {
        return HAL_ERR_INVALID;
    }

    hal_timer_entry_t *entry = &hal_timers[timer_id];
    if (!entry->handle) {
        return HAL_OK;
    }

    esp_timer_stop(entry->handle);
    esp_timer_delete(entry->handle);
    entry->handle = NULL;
    entry->callback = NULL;
    entry->user_data = NULL;
    return HAL_OK;
}

static esp_timer_handle_t hal_tick_timer;
static hal_timer_callback_t hal_tick_callback;
static void *hal_tick_user_data;

static void hal_tick_dispatch(void *arg) {
    (void)arg;
    if (hal_tick_callback) {
        hal_tick_callback(hal_tick_user_data);
    }
}

hal_status_t hal_tick_start(uint32_t hz, hal_timer_callback_t callback, void *user_data) {
    if (hz == 0 || callback == NULL) {
        return HAL_ERR_INVALID;
    }

    if (hal_tick_timer) {
        esp_timer_stop(hal_tick_timer);
        esp_timer_delete(hal_tick_timer);
        hal_tick_timer = NULL;
    }

    uint64_t period_us = 1000000ULL / hz;
    if (period_us == 0) {
        return HAL_ERR_INVALID;
    }

    esp_timer_create_args_t args = {
        .callback = &hal_tick_dispatch,
        .arg = NULL,
        .name = "hal_tick"
    };
    if (esp_timer_create(&args, &hal_tick_timer) != ESP_OK) {
        hal_tick_timer = NULL;
        return HAL_ERR_INVALID;
    }

    hal_tick_callback = callback;
    hal_tick_user_data = user_data;

    if (esp_timer_start_periodic(hal_tick_timer, period_us) != ESP_OK) {
        esp_timer_delete(hal_tick_timer);
        hal_tick_timer = NULL;
        return HAL_ERR_INVALID;
    }

    return HAL_OK;
}

hal_status_t hal_tick_stop(void) {
    if (!hal_tick_timer) {
        return HAL_OK;
    }
    esp_timer_stop(hal_tick_timer);
    esp_timer_delete(hal_tick_timer);
    hal_tick_timer = NULL;
    hal_tick_callback = NULL;
    hal_tick_user_data = NULL;
    return HAL_OK;
}

static ledc_timer_t hal_pwm_select_timer(int pin) {
    if (pin == BOARD_GPIO_MOTOR_IN1 || pin == BOARD_GPIO_MOTOR_IN2) {
        return LEDC_TIMER_1;
    }
    return LEDC_TIMER_0;
}

static ledc_timer_bit_t hal_pwm_bits_to_ledc(uint32_t bits) {
    if (bits < 1) {
        bits = 1;
    }
    if (bits >= LEDC_TIMER_BIT_MAX) {
        bits = LEDC_TIMER_BIT_MAX - 1;
    }
    return (ledc_timer_bit_t)bits;
}

typedef struct {
    bool configured;
    int pin;
    ledc_timer_t timer;
    uint32_t duty_max;
} hal_pwm_channel_t;

static hal_pwm_channel_t hal_pwm_channels[LEDC_CHANNEL_MAX];
static bool hal_pwm_timer_configured[LEDC_TIMER_MAX];
static uint32_t hal_pwm_timer_freq[LEDC_TIMER_MAX];
static uint32_t hal_pwm_timer_bits[LEDC_TIMER_MAX];

hal_status_t hal_pwm_init(int channel, int pin, uint32_t freq_hz,
                          uint32_t duty_resolution_bits) {
    if (channel < 0 || channel >= LEDC_CHANNEL_MAX) {
        return HAL_ERR_INVALID;
    }
    if (!hal_gpio_output_valid(pin)) {
        return HAL_ERR_INVALID;
    }

    ledc_timer_t timer = hal_pwm_select_timer(pin);
    ledc_timer_bit_t res = hal_pwm_bits_to_ledc(duty_resolution_bits);

    if (!hal_pwm_timer_configured[timer] ||
        hal_pwm_timer_freq[timer] != freq_hz ||
        hal_pwm_timer_bits[timer] != duty_resolution_bits) {
        ledc_timer_config_t timer_cfg = {
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .timer_num = timer,
            .duty_resolution = res,
            .freq_hz = freq_hz,
            .clk_cfg = LEDC_AUTO_CLK,
        };
        if (ledc_timer_config(&timer_cfg) != ESP_OK) {
            return HAL_ERR_INVALID;
        }
        hal_pwm_timer_configured[timer] = true;
        hal_pwm_timer_freq[timer] = freq_hz;
        hal_pwm_timer_bits[timer] = duty_resolution_bits;
    }

    ledc_channel_config_t channel_cfg = {
        .gpio_num = pin,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = (ledc_channel_t)channel,
        .timer_sel = timer,
        .duty = 0,
        .hpoint = 0,
    };
    if (ledc_channel_config(&channel_cfg) != ESP_OK) {
        return HAL_ERR_INVALID;
    }

    hal_pwm_channels[channel].configured = true;
    hal_pwm_channels[channel].pin = pin;
    hal_pwm_channels[channel].timer = timer;
    if (duty_resolution_bits >= 31) {
        hal_pwm_channels[channel].duty_max = 0xFFFFFFFFU;
    } else {
        hal_pwm_channels[channel].duty_max = (1U << duty_resolution_bits) - 1U;
    }

    return HAL_OK;
}

hal_status_t hal_pwm_set_duty(int channel, uint32_t duty) {
    if (channel < 0 || channel >= LEDC_CHANNEL_MAX) {
        return HAL_ERR_INVALID;
    }
    if (!hal_pwm_channels[channel].configured) {
        return HAL_ERR_UNSUPPORTED;
    }

    uint32_t max_duty = hal_pwm_channels[channel].duty_max;
    if (duty > max_duty) {
        duty = max_duty;
    }

    if (ledc_set_duty(LEDC_LOW_SPEED_MODE, (ledc_channel_t)channel, duty) != ESP_OK) {
        return HAL_ERR_INVALID;
    }
    if (ledc_update_duty(LEDC_LOW_SPEED_MODE, (ledc_channel_t)channel) != ESP_OK) {
        return HAL_ERR_INVALID;
    }
    return HAL_OK;
}

hal_status_t hal_uart_init(int uart_id, uint32_t baud_rate) {
    if (uart_id != UART_NUM_0) {
        return HAL_ERR_UNSUPPORTED;
    }

    if (HAL_UART0_TX_PIN == BOARD_GPIO_UNUSED || HAL_UART0_RX_PIN == BOARD_GPIO_UNUSED) {
        return HAL_ERR_UNSUPPORTED;
    }

    if (!hal_gpio_valid(HAL_UART0_TX_PIN) || !hal_gpio_valid(HAL_UART0_RX_PIN)) {
        return HAL_ERR_INVALID;
    }

    uart_config_t config = {
        .baud_rate = (int)baud_rate,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    if (uart_param_config(uart_id, &config) != ESP_OK) {
        return HAL_ERR_INVALID;
    }

    int rts = (HAL_UART0_RTS_PIN == BOARD_GPIO_UNUSED) ? UART_PIN_NO_CHANGE : HAL_UART0_RTS_PIN;
    int cts = (HAL_UART0_CTS_PIN == BOARD_GPIO_UNUSED) ? UART_PIN_NO_CHANGE : HAL_UART0_CTS_PIN;
    if (uart_set_pin(uart_id, HAL_UART0_TX_PIN, HAL_UART0_RX_PIN, rts, cts) != ESP_OK) {
        return HAL_ERR_INVALID;
    }

    if (uart_driver_install(uart_id, 256, 0, 0, NULL, 0) != ESP_OK) {
        return HAL_ERR_INVALID;
    }

    return HAL_OK;
}

int hal_uart_write(int uart_id, const uint8_t *data, size_t length) {
    if (uart_id != UART_NUM_0 || data == NULL || length == 0) {
        return HAL_ERR_INVALID;
    }
    return uart_write_bytes(uart_id, data, length);
}

int hal_uart_read(int uart_id, uint8_t *data, size_t length) {
    if (uart_id != UART_NUM_0 || data == NULL || length == 0) {
        return HAL_ERR_INVALID;
    }
    return uart_read_bytes(uart_id, data, length, 0);
}

static bool hal_i2c_initialized;

hal_status_t hal_i2c_init(int bus_id, uint32_t clock_hz) {
    if (bus_id != I2C_NUM_0) {
        return HAL_ERR_UNSUPPORTED;
    }
    if (HAL_I2C0_SDA_PIN == BOARD_GPIO_UNUSED || HAL_I2C0_SCL_PIN == BOARD_GPIO_UNUSED) {
        return HAL_ERR_UNSUPPORTED;
    }
    if (!hal_gpio_valid(HAL_I2C0_SDA_PIN) || !hal_gpio_valid(HAL_I2C0_SCL_PIN)) {
        return HAL_ERR_INVALID;
    }
    if (hal_i2c_initialized) {
        return HAL_OK;
    }

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = HAL_I2C0_SDA_PIN,
        .scl_io_num = HAL_I2C0_SCL_PIN,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = (clock_hz == 0) ? HAL_I2C0_CLOCK_HZ : clock_hz,
    };

    if (i2c_param_config(bus_id, &conf) != ESP_OK) {
        return HAL_ERR_INVALID;
    }
    if (i2c_driver_install(bus_id, conf.mode, 0, 0, 0) != ESP_OK) {
        return HAL_ERR_INVALID;
    }

    hal_i2c_initialized = true;
    return HAL_OK;
}

int hal_i2c_write(int bus_id, uint8_t addr, const uint8_t *data, size_t length) {
    if (data == NULL || length == 0) {
        return HAL_ERR_INVALID;
    }
    if (hal_i2c_init(bus_id, HAL_I2C0_CLOCK_HZ) != HAL_OK) {
        return HAL_ERR_UNSUPPORTED;
    }
    esp_err_t err = i2c_master_write_to_device(bus_id, addr, data, length,
                                               pdMS_TO_TICKS(100));
    return (err == ESP_OK) ? (int)length : HAL_ERR_INVALID;
}

int hal_i2c_read(int bus_id, uint8_t addr, uint8_t *data, size_t length) {
    if (data == NULL || length == 0) {
        return HAL_ERR_INVALID;
    }
    if (hal_i2c_init(bus_id, HAL_I2C0_CLOCK_HZ) != HAL_OK) {
        return HAL_ERR_UNSUPPORTED;
    }
    esp_err_t err = i2c_master_read_from_device(bus_id, addr, data, length,
                                                pdMS_TO_TICKS(100));
    return (err == ESP_OK) ? (int)length : HAL_ERR_INVALID;
}

static bool hal_spi_initialized;
static spi_device_handle_t hal_spi_device;

#ifdef HAL_SELFTEST
static void hal_selftest_tick_cb(void *ctx) {
    volatile uint32_t *count = (volatile uint32_t *)ctx;
    (*count)++;
}
#endif

hal_status_t hal_spi_init(int bus_id, uint32_t clock_hz, uint8_t mode) {
    if (bus_id != 0) {
        return HAL_ERR_UNSUPPORTED;
    }
    if (HAL_SPI0_MOSI_PIN == BOARD_GPIO_UNUSED || HAL_SPI0_MISO_PIN == BOARD_GPIO_UNUSED ||
        HAL_SPI0_SCLK_PIN == BOARD_GPIO_UNUSED) {
        return HAL_ERR_UNSUPPORTED;
    }
    if (!hal_gpio_valid(HAL_SPI0_MOSI_PIN) || !hal_gpio_valid(HAL_SPI0_MISO_PIN) ||
        !hal_gpio_valid(HAL_SPI0_SCLK_PIN)) {
        return HAL_ERR_INVALID;
    }
    if (hal_spi_initialized) {
        return HAL_OK;
    }

    spi_bus_config_t buscfg = {
        .mosi_io_num = HAL_SPI0_MOSI_PIN,
        .miso_io_num = HAL_SPI0_MISO_PIN,
        .sclk_io_num = HAL_SPI0_SCLK_PIN,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4096,
    };

    if (spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO) != ESP_OK) {
        return HAL_ERR_INVALID;
    }

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = (clock_hz == 0) ? HAL_SPI0_CLOCK_HZ : clock_hz,
        .mode = mode,
        .spics_io_num = (HAL_SPI0_CS_PIN == BOARD_GPIO_UNUSED) ? -1 : HAL_SPI0_CS_PIN,
        .queue_size = 1,
    };

    if (spi_bus_add_device(SPI2_HOST, &devcfg, &hal_spi_device) != ESP_OK) {
        return HAL_ERR_INVALID;
    }

    hal_spi_initialized = true;
    return HAL_OK;
}

int hal_spi_transfer(int bus_id, const uint8_t *tx, uint8_t *rx, size_t length) {
    if (length == 0) {
        return HAL_ERR_INVALID;
    }
    if (hal_spi_init(bus_id, HAL_SPI0_CLOCK_HZ, HAL_SPI0_MODE) != HAL_OK) {
        return HAL_ERR_UNSUPPORTED;
    }

    spi_transaction_t t = {
        .length = length * 8,
        .tx_buffer = tx,
        .rx_buffer = rx,
    };

    esp_err_t err = spi_device_transmit(hal_spi_device, &t);
    return (err == ESP_OK) ? (int)length : HAL_ERR_INVALID;
}

static adc_oneshot_unit_handle_t hal_adc_handle;
static bool hal_adc_initialized;

hal_status_t hal_adc_init(void) {
    if (hal_adc_initialized) {
        return HAL_OK;
    }

    adc_oneshot_unit_init_cfg_t init_cfg = {
        .unit_id = ADC_UNIT_1,
    };
    if (adc_oneshot_new_unit(&init_cfg, &hal_adc_handle) != ESP_OK) {
        return HAL_ERR_INVALID;
    }

    hal_adc_initialized = true;
    return HAL_OK;
}

int hal_adc_read(int channel) {
    if (channel == BOARD_GPIO_UNUSED) {
        return HAL_ERR_UNSUPPORTED;
    }
    if (channel < 0) {
        return HAL_ERR_INVALID;
    }

    if (hal_adc_init() != HAL_OK) {
        return HAL_ERR_UNSUPPORTED;
    }

    adc_oneshot_chan_cfg_t cfg = {
        .atten = ADC_ATTEN_DB_11,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    if (adc_oneshot_config_channel(hal_adc_handle, (adc_channel_t)channel, &cfg) != ESP_OK) {
        return HAL_ERR_INVALID;
    }

    int raw = 0;
    if (adc_oneshot_read(hal_adc_handle, (adc_channel_t)channel, &raw) != ESP_OK) {
        return HAL_ERR_INVALID;
    }
    return raw;
}

void hal_selftest_run(void) {
#ifndef HAL_SELFTEST
    return;
#else
    ESP_LOGI(TAG, "HAL self-test start");

    int led_pins[] = {
        BOARD_GPIO_LED0,
        BOARD_GPIO_LED1,
        BOARD_GPIO_LED2,
        BOARD_GPIO_LED3
    };

    for (size_t i = 0; i < sizeof(led_pins) / sizeof(led_pins[0]); i++) {
        if (hal_gpio_config_output(led_pins[i], HAL_GPIO_LOW) == HAL_OK) {
            hal_gpio_write(led_pins[i], HAL_GPIO_HIGH);
            hal_delay_ms(200);
            hal_gpio_write(led_pins[i], HAL_GPIO_LOW);
        }
    }

    hal_gpio_config_input(BOARD_GPIO_BUTTON_FWD, HAL_GPIO_PULL_UP);
    hal_gpio_config_input(BOARD_GPIO_BUTTON_REV, HAL_GPIO_PULL_UP);
    ESP_LOGI(TAG, "Button FWD=%d REV=%d",
             hal_gpio_read(BOARD_GPIO_BUTTON_FWD),
             hal_gpio_read(BOARD_GPIO_BUTTON_REV));

    volatile uint32_t tick_count = 0;
    hal_tick_start(1000, hal_selftest_tick_cb, (void *)&tick_count);
    hal_delay_ms(1100);
    hal_tick_stop();
    ESP_LOGI(TAG, "Tick count (1s): %u", (unsigned)tick_count);

    hal_pwm_init(0, BOARD_GPIO_LED0, HAL_PWM_LED_FREQ_HZ, HAL_PWM_DUTY_RES_BITS);
    hal_pwm_set_duty(0, (1U << (HAL_PWM_DUTY_RES_BITS - 1)));

    if (hal_uart_init(UART_NUM_0, HAL_UART0_BAUD_DEFAULT) == HAL_OK) {
        const char banner[] = "HAL UART0 ready\n";
        hal_uart_write(UART_NUM_0, (const uint8_t *)banner, sizeof(banner) - 1);
    }

    if (HAL_I2C0_SDA_PIN != BOARD_GPIO_UNUSED && HAL_I2C0_SCL_PIN != BOARD_GPIO_UNUSED) {
        hal_i2c_init(I2C_NUM_0, HAL_I2C0_CLOCK_HZ);
    }

    if (HAL_SPI0_MOSI_PIN != BOARD_GPIO_UNUSED && HAL_SPI0_SCLK_PIN != BOARD_GPIO_UNUSED) {
        hal_spi_init(0, HAL_SPI0_CLOCK_HZ, HAL_SPI0_MODE);
    }

    if (HAL_ADC_DEFAULT_CHANNEL != BOARD_GPIO_UNUSED) {
        int adc_value = hal_adc_read(HAL_ADC_DEFAULT_CHANNEL);
        ESP_LOGI(TAG, "ADC default channel=%d value=%d", HAL_ADC_DEFAULT_CHANNEL, adc_value);
    }

    ESP_LOGI(TAG, "HAL self-test complete");
#endif
}
