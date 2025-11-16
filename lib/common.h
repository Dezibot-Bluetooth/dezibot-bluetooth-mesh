#ifndef DEZIBOT_BLUETOOTH_MESH_COMMON_H
#define DEZIBOT_BLUETOOTH_MESH_COMMON_H

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

// Bluetooth Mesh APIs
#include "esp_ble_mesh_defs.h"
#include "esp_ble_mesh_common_api.h"
#include "esp_ble_mesh_provisioning_api.h"
#include "esp_ble_mesh_networking_api.h"
#include "esp_ble_mesh_config_model_api.h"
#include "esp_ble_mesh_generic_model_api.h"

#endif //DEZIBOT_BLUETOOTH_MESH_COMMON_H