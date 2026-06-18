#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "board.h"
#include "esp_err.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "ui_core.h"

static const char *TAG = "main";

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if ((ret == ESP_ERR_NVS_NO_FREE_PAGES) || (ret == ESP_ERR_NVS_NEW_VERSION_FOUND)) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "Booting NECTAR premium UI foundation");

    ESP_ERROR_CHECK(board_init());
    ESP_ERROR_CHECK(ui_core_init());

    ESP_LOGI(TAG, "NECTAR UI runtime initialized");
    vTaskDelete(NULL);
}
