#include "common.h"
#include "server.h"
#include "bluetooth.h"
#include "init.h"

#define TAG "MAIN"

void app_main(void)
{
    ESP_LOGI(TAG, "Starting BLE Mesh Server...");

    ESP_ERROR_CHECK(pre_init());
    ESP_ERROR_CHECK(ble_mesh_server_init());
}
