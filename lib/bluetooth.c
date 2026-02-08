#include "bluetooth.h"

#define TAG "BLUETOOTH"

static SemaphoreHandle_t mesh_sem;
static uint8_t own_addr_type;
void ble_store_config_init(void);
static uint8_t addr_val[6] = {0};

void ble_mesh_get_dev_uuid(uint8_t *dev_uuid)
{
    if (dev_uuid == NULL)
    {
        ESP_LOGE(TAG, "%s, Invalid device uuid", __func__);
        return;
    }

    memcpy(dev_uuid + 2, addr_val, BD_ADDR_LEN);
}

static void mesh_on_reset(int reason)
{
    ESP_LOGI(TAG, "Resetting state - reason=%d", reason);
}

static void mesh_on_sync(void)
{
    esp_err_t error;

    error = ble_hs_util_ensure_addr(0);
    assert(error == 0);

    error = ble_hs_id_infer_auto(0, &own_addr_type);
    if (error != 0) {
        ESP_LOGI(TAG, "error determining address type - error=%d", error);
        return;
    }

    error = ble_hs_id_copy_addr(own_addr_type, addr_val, NULL);

    xSemaphoreGive(mesh_sem);
}

void mesh_host_task(void *param)
{
    ESP_LOGI(TAG, "BLE Host Task Started");
    nimble_port_run();

    nimble_port_freertos_deinit();
}

esp_err_t bluetooth_init(void)
{
    esp_err_t error;

    mesh_sem = xSemaphoreCreateBinary();
    if (mesh_sem == NULL) {
        ESP_LOGE(TAG, "Failed to create mesh semaphore");
        return ESP_FAIL;
    }

    error = nimble_port_init();
    if (error != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init nimble %d ", error);
        return error;
    }

    ble_hs_cfg.reset_cb = mesh_on_reset;
    ble_hs_cfg.sync_cb = mesh_on_sync;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

    ble_store_config_init();

    nimble_port_freertos_init(mesh_host_task);

    xSemaphoreTake(mesh_sem, portMAX_DELAY);

    return ESP_OK;
}