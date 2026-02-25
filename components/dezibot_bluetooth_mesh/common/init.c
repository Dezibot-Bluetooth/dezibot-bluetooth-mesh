/**
 * @file init.c
 * @brief System initialization implementation
 * 
 * Orchestrates the initialization sequence for NVS flash and Bluetooth stack.
 * 
 * @author DeziBot Project
 * @date 2026
 */

#include "init.h"
#include "common.h"
#include "bluetooth.h"

#include <stdbool.h>

#define TAG "INIT"  /**< Logging tag for this module */

// Arduino-ESP32's initArduino() will release BT controller memory unless
// btInUse() returns true. This project uses ESP-IDF NimBLE directly, so we
// must prevent that early memory release.
bool btInUse(void)
{
    return true;
}

/**
 * @brief Perform pre-application initialization
 * 
 * Initializes core system components in the correct order:
 * 1. NVS (Non-Volatile Storage) flash
 * 2. Bluetooth stack
 * 
 * NVS Error Handling:
 * - ESP_ERR_NVS_NO_FREE_PAGES: Flash is full, erase and reinitialize
 * - Other NVS errors: Halt execution via ESP_ERROR_CHECK
 * 
 * Bluetooth Errors:
 * - Logged and returned to caller for handling
 * - Application can decide whether to continue or halt
 * 
 * @return 
 *   - ESP_OK: All initialization successful
 *   - Error codes from bluetooth_init() on failure
 * 
 * @note NVS must be initialized before Bluetooth (stores bonding info)
 * @note Fatal NVS errors halt execution
 * @note Bluetooth errors are returned for application handling
 * 
 * @see bluetooth_init()
 */
esp_err_t pre_init(void)
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