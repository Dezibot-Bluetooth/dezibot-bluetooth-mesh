#ifndef DEZIBOT_BLUETOOTH_MESH_NODE_H
#define DEZIBOT_BLUETOOTH_MESH_NODE_H

#include "common/common.h"
#include "server/server_events.h"

/**
 * @brief Initialize BLE Mesh node (client + server).
 *
 * Initializes a node with both client and server models enabled and registers
 * the application callback for incoming server events.
 *
 * @param[in] device_name Device name to expose via GAP.
 * @param[in] cb Application callback for incoming server events.
 *
 * @return
 *   - ESP_OK on success
 *   - ESP_FAIL / other esp_err_t on failure
 */
esp_err_t ble_mesh_node_init(char *device_name, mesh_server_evt_cb_t cb);

#endif //DEZIBOT_BLUETOOTH_MESH_NODE_H
