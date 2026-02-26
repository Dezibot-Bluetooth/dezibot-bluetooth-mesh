#include "init.h"
#include "common.h"
#include "bluetooth.h"

#include <stdbool.h>

#define TAG "INIT"  /**< Logging tag for this module */

bool btInUse(void)
{
    return true;
}

esp_err_t pre_init()
{
    esp_err_t error;

    error = nvs_flash_init();
    if (error == ESP_ERR_NVS_NO_FREE_PAGES)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        error = nvs_flash_init();
    }
    ESP_ERROR_CHECK(error);

    error = bluetooth_init();
    if (error)
    {
        ESP_LOGE(TAG, "esp32_bluetooth_init failed (err %d)", error);
        return error;
    }

    return ESP_OK;
}
