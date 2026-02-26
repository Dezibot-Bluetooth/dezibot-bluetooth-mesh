/**
 * @file bluetooth.h
 * @brief Bluetooth stack initialization and device identification
 * 
 * This module provides low-level Bluetooth stack initialization using the
 * NimBLE stack and manages device UUID generation based on MAC address.
 * 
 * Key Features:
 * - NimBLE stack initialization with FreeRTOS integration
 * - Binary semaphore synchronization for init completion
 * - Device UUID generation from Bluetooth MAC address
 * - Bluetooth host task management
 */

#ifndef DEZIBOT_BLUETOOTH_MESH_MESH_INIT_H
#define DEZIBOT_BLUETOOTH_MESH_MESH_INIT_H

#include "common.h"

/**
 * @brief Generate device UUID from Bluetooth MAC address
 * 
 * Generates a 16-byte UUID with the device's Bluetooth MAC address
 * embedded in bytes 2-7. This provides a deterministic, unique
 * identifier for each device.
 * 
 * @param[out] dev_uuid Pointer to 16-byte buffer for UUID
 * 
 * @note The caller must provide a buffer of at least 16 bytes
 * @warning Function logs error and returns if dev_uuid is NULL
 */
void ble_mesh_get_dev_uuid(uint8_t *dev_uuid);

/**
 * @brief Initialize Bluetooth stack
 * 
 * Initializes the NimBLE Bluetooth stack and starts the host task.
 * This function blocks until the stack synchronization is complete,
 * ensuring Bluetooth is ready before returning.
 * 
 * Initialization sequence:
 * 1. Create binary semaphore for synchronization
 * 2. Initialize NimBLE port
 * 3. Configure host callbacks (reset, sync, storage)
 * 4. Initialize BLE storage configuration
 * 5. Start FreeRTOS host task
 * 6. Wait for sync callback (semaphore)
 * 
 * @return 
 *   - ESP_OK: Bluetooth initialized successfully
 *   - ESP_FAIL: Failed to create semaphore
 *   - Other error codes from nimble_port_init()
 * 
 * @note This function must be called before any mesh operations
 * @note Blocks until Bluetooth host is synchronized
 */
esp_err_t bluetooth_init();

#endif //DEZIBOT_BLUETOOTH_MESH_MESH_INIT_H