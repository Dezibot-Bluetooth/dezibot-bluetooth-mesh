/**
 * @file server.h
 * @brief BLE Mesh server API.
 */

#ifndef DEZIBOT_BLUETOOTH_MESH_SERVER_H
#define DEZIBOT_BLUETOOTH_MESH_SERVER_H

#include "common/common.h"
#include "server/server_events.h"

/**
 * @brief Override the default composition data.
 *
 * Call this before @ref ble_mesh_server_init if you want to provide a custom
 * element/model composition.
 *
 * @param[in] comp Composition data to use.
 */
void ble_mesh_server_set_composition(esp_ble_mesh_comp_t *comp);

/**
 * @brief BLE Mesh Generic Server callback handler.
 *
 * Internal callback passed to the ESP BLE Mesh stack for Generic Server model
 * events. Applications typically register their handler via
 * @ref ble_mesh_server_init.
 *
 * @param[in] event Event ID from the ESP BLE Mesh stack.
 * @param[in] param Event parameters provided by the ESP BLE Mesh stack.
 */
void mesh_generic_server_cb(esp_ble_mesh_generic_server_cb_event_t event, esp_ble_mesh_generic_server_cb_param_t *param);

/**
 * @brief BLE Mesh Configuration Server callback handler.
 *
 * Internal callback passed to the ESP BLE Mesh stack for Configuration Server
 * model events.
 *
 * @param[in] event Event ID from the ESP BLE Mesh stack.
 * @param[in] param Event parameters provided by the ESP BLE Mesh stack.
 */
void mesh_config_server_cb(esp_ble_mesh_cfg_server_cb_event_t event, esp_ble_mesh_cfg_server_cb_param_t *param);

/**
 * @brief Initialize BLE Mesh server models.
 *
 * Registers callbacks and initializes the server-side models. The application
 * callback is invoked for incoming server events.
 *
 * @param[in] device_name Device name to expose via GAP.
 * @param[in] cb Application callback for incoming server events.
 *
 * @return
 *   - ESP_OK on success
 *   - ESP_FAIL / other esp_err_t on failure
 */
esp_err_t ble_mesh_server_init(char *device_name, mesh_server_evt_cb_t cb);

#endif //DEZIBOT_BLUETOOTH_MESH_SERVER_H
