/**
 * @file init.h
 * @brief System initialization orchestration
 * 
 * This module provides a unified entry point for all system initialization
 * tasks, ensuring components start in the correct order.
 * 
 * Initialization Sequence:
 * 1. NVS (Non-Volatile Storage) flash initialization
 * 2. NVS error handling and recovery
 * 3. Bluetooth stack initialization
 * 
 * @author DeziBot Project
 * @date 2026
 */

#ifndef DEZIBOT_BLUETOOTH_MESH_INIT_H
#define DEZIBOT_BLUETOOTH_MESH_INIT_H

#include "common.h"

/**
 * @brief Perform pre-application initialization
 * 
 * Orchestrates the initialization of core system components required
 * before application logic can run. This includes NVS flash setup
 * and Bluetooth stack initialization.
 * 
 * NVS Error Handling:
 * - If NVS has no free pages, flash is erased and reinitialized
 * - Critical errors halt execution via ESP_ERROR_CHECK
 * 
 * @return 
 *   - ESP_OK: All initialization successful
 *   - Error codes from bluetooth_init() on Bluetooth failures
 * 
 * @note Call this before any mesh or application operations
 * @note Fatal errors will halt execution
 * 
 * @see bluetooth_init()
 */
esp_err_t pre_init(void);

#endif //DEZIBOT_BLUETOOTH_MESH_INIT_H