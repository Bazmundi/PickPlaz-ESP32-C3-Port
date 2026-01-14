#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hal.h"
#include "pickplaz_app.h"

static const char *TAG = "pickplaz";

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
