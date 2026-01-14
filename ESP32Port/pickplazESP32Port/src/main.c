#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hal.h"

static const char *TAG = "pickplaz";

void app_main(void) {
    ESP_LOGI(TAG, "Booting PickPlaz ESP32-C3 port (Stage 1)");
    ESP_LOGI(TAG, "IDF version: %s", esp_get_idf_version());
    hal_init();

    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
