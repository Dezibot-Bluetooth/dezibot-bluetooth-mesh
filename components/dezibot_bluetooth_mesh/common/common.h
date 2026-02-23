/**
 * @file common.h
 * @brief Common header file for DeziBot Bluetooth Mesh library
 * 
 * This header provides centralized access to all required APIs for the
 * Bluetooth Mesh implementation. It includes standard C libraries, ESP-IDF
 * core APIs, FreeRTOS primitives, NimBLE stack headers, and BLE Mesh APIs.
 * 
 * Design Pattern: Central Include Pattern
 * This reduces code duplication and ensures consistent API access across
 * all modules. Modules only need to include this single header.
 * 
 * @author DeziBot Project
 * @date 2026
 */

#ifndef DEZIBOT_BLUETOOTH_MESH_COMMON_H
#define DEZIBOT_BLUETOOTH_MESH_COMMON_H

/**
 * @file common.h
 * @brief Common includes and types for the BLE Mesh modules.
 */

// STD APIs
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

// ESP APIs
#include "esp_err.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "nvs_flash.h"

// FreeRTOS APIs
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// NimBLE APIs
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "console/console.h"
#include "services/gap/ble_svc_gap.h"

// Bluetooth Mesh APIs
#include "esp_ble_mesh_defs.h"
#include "esp_ble_mesh_common_api.h"
#include "esp_ble_mesh_provisioning_api.h"
#include "esp_ble_mesh_networking_api.h"
#include "esp_ble_mesh_config_model_api.h"
#include "esp_ble_mesh_generic_model_api.h"

#endif //DEZIBOT_BLUETOOTH_MESH_COMMON_H
