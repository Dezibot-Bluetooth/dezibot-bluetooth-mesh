/**
 * @file bluetooth.c
 * @brief Bluetooth stack initialization implementation
 * 
 * Implements NimBLE stack initialization with FreeRTOS integration and
 * provides device UUID generation from Bluetooth MAC address.
 * 
 * @author DeziBot Project
 * @date 2026
 */

#include "bluetooth.h"

#define TAG "BLUETOOTH"  /**< Logging tag for this module */

static SemaphoreHandle_t mesh_sem;  /**< Binary semaphore for init synchronization */
static uint8_t own_addr_type;       /**< Bluetooth address type (public/random) */
void ble_store_config_init(void);   /**< External function from NimBLE */
static uint8_t addr_val[6] = {0};   /**< Bluetooth MAC address storage */

/**
 * @brief Generate device UUID from Bluetooth MAC address
 * 
 * Creates a 16-byte UUID with the device's Bluetooth MAC address
 * embedded at bytes 2-7. This provides a deterministic unique
 * identifier that remains constant across reboots.
 * 
 * UUID Structure:
 * - Bytes 0-1: Uninitialized (may contain garbage)
 * - Bytes 2-7: Bluetooth MAC address
 * - Bytes 8-15: Uninitialized (may contain garbage)
 * 
 * @param[out] dev_uuid Pointer to 16-byte buffer for UUID output
 * 
 * @note Logs error and returns if dev_uuid is NULL
 * @note MAC address must be retrieved first via mesh_on_sync()
 * 
 * @see mesh_on_sync()
 */
void ble_mesh_get_dev_uuid(uint8_t *dev_uuid)
{
    if (dev_uuid == NULL)
    {
        ESP_LOGE(TAG, "%s, Invalid device uuid", __func__);
        return;
    }

    memcpy(dev_uuid + 2, addr_val, BD_ADDR_LEN);
}

/**
 * @brief Bluetooth stack reset callback
 * 
 * Called by NimBLE when the stack resets. This typically occurs
 * on fatal errors or explicit reset requests.
 * 
 * @param[in] reason Reset reason code
 * 
 * @note Currently only logs the reset event
 * @note Application should implement recovery logic if needed
 */
static void mesh_on_reset(int reason)
{
    ESP_LOGI(TAG, "Resetting state - reason=%d", reason);
}

/**
 * @brief Bluetooth stack synchronization callback
 * 
 * Called by NimBLE when the host is synchronized and ready.
 * This callback retrieves the device's Bluetooth address and
 * signals the initialization semaphore.
 * 
 * Actions performed:
 * 1. Ensure Bluetooth address is available
 * 2. Infer address type (public/random)
 * 3. Copy address to addr_val for UUID generation
 * 4. Signal mesh_sem to unblock bluetooth_init()
 * 
 * @note This runs in the BLE host task context
 * @note Errors in address determination are logged but don't block
 */
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

/**
 * @brief NimBLE host task
 * 
 * FreeRTOS task that runs the NimBLE host stack. This is a blocking
 * call that processes BLE events until the stack is shut down.
 * 
 * Task flow:
 * 1. Log task start
 * 2. Run NimBLE host (blocking)
 * 3. Deinitialize FreeRTOS integration on exit
 * 
 * @param[in] param Task parameter (unused)
 * 
 * @note This task is created automatically by nimble_port_freertos_init()
 * @note Task runs until nimble_port_stop() is called
 */
void mesh_host_task(void *param)
{
    ESP_LOGI(TAG, "BLE Host Task Started");
    nimble_port_run();

    nimble_port_freertos_deinit();
}

/**
 * @brief Initialize Bluetooth stack
 * 
 * Initializes the NimBLE Bluetooth stack and waits for synchronization.
 * This function blocks until the stack is ready for mesh operations.
 * 
 * Initialization sequence:
 * 1. Create binary semaphore for synchronization
 * 2. Initialize NimBLE port
 * 3. Configure host callbacks:
 *    - reset_cb: mesh_on_reset()
 *    - sync_cb: mesh_on_sync()
 *    - store_status_cb: ble_store_util_status_rr()
 * 4. Initialize BLE storage configuration
 * 5. Create and start host task via nimble_port_freertos_init()
 * 6. Wait for sync callback (blocks on semaphore)
 * 
 * Synchronization Pattern:
 * The function creates a binary semaphore, starts the BLE host task,
 * and blocks on the semaphore. When the host is ready, mesh_on_sync()
 * gives the semaphore, allowing this function to return.
 * 
 * @return 
 *   - ESP_OK: Bluetooth initialized and synchronized
 *   - ESP_FAIL: Failed to create semaphore
 *   - Other: Error from nimble_port_init()
 * 
 * @note This function blocks until Bluetooth is ready
 * @note Must be called before any mesh operations
 * @note Stores Bluetooth MAC address for UUID generation
 * 
 * @see mesh_on_sync()
 * @see ble_mesh_get_dev_uuid()
 */
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
