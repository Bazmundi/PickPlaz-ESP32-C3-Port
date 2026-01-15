/**
 * @file main.c
 * @brief Entry point for the PickPlaz ESP32-C3 firmware.
 *
 * @details
 * Owns firmware startup sequencing: HAL bring-up, optional self-test, and
 * application start. This file is the module boundary between ESP-IDF
 * startup and PickPlaz application logic.
 */

#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hal.h"
#include "pickplaz_app.h"

static const char *TAG = "pickplaz";

/**
 * @brief Starts the PickPlaz firmware and application loop.
 *
 * @details
 * Logs boot metadata, initializes HAL services, runs any enabled self-test,
 * and starts the PickPlaz application state machine.
 *
 * Preconditions:
 * - ESP-IDF runtime is initialized.
 *
 * Postconditions:
 * - HAL services are initialized.
 * - The application tick is running.
 *
 * Side effects:
 * - Writes boot and status logs to the console.
 * - Configures GPIO/PWM/ADC resources through the HAL.
 *
 * Error handling:
 * - Errors are reported via logs in lower layers; this function does not
 *   return error codes.
 *
 * @note The self-test runs only when HAL_SELFTEST is defined.
 *
 * @par Inputs/Outputs
 * | Item   | Description |
 * | ------ | ----------- |
 * | Inputs | Build flags, board pin mappings, device peripherals |
 * | Outputs | Log messages and initialized runtime services |
 */
void app_main(void) {
    ESP_LOGI(TAG, "Booting PickPlaz ESP32-C3 port (Stage 4)");
    ESP_LOGI(TAG, "IDF version: %s", esp_get_idf_version());
    hal_init();
#ifdef HAL_SELFTEST
    hal_selftest_run();
#endif
    pickplaz_app_init();
    pickplaz_app_start();

    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
