#include "esp_log.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lib/client.h"
#include "lib/bluetooth.h"

#define TAG "MAIN"

void app_main(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    ESP_LOGI(TAG, "Starting BLE Mesh Client...");

    ESP_ERROR_CHECK(bluetooth_init());
    ESP_ERROR_CHECK(ble_mesh_client_init());

    // Give provisioning time to complete
    vTaskDelay(pdMS_TO_TICKS(5000));

    while (1) {
        ble_mesh_client_send(1, 0x0002);
        vTaskDelay(pdMS_TO_TICKS(2000));
        ble_mesh_client_send(0, 0x0002);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
