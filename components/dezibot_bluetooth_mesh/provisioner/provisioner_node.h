/**
 * @file provisioner_node.h
 * @brief Combined provisioner + node initialization
 *
 * This module initializes a single device as both a BLE Mesh provisioner
 * AND a full node (client + server). Because esp_ble_mesh_init() can only
 * be called once, the composition data merges provisioner models (CFG_SRV,
 * CFG_CLI) with all generic client and server models into one element.
 *
 * After initialization the device:
 * - Is self-provisioned at address PROV_OWN_ADDR (0x0001)
 * - Can discover and provision other devices (UUID prefix 0xdd 0xdd)
 * - Can send generic client messages (OnOff, Level, etc.)
 * - Can receive generic server messages (OnOff, Level, etc.)
 *
 * @author DeziBot Project
 * @date 2026
 */

#ifndef DEZIBOT_BLUETOOTH_MESH_PROVISIONER_NODE_H
#define DEZIBOT_BLUETOOTH_MESH_PROVISIONER_NODE_H

#include "../common/common.h"
#include "../server/server_events.h"

/**
 * @brief Initialize BLE Mesh as combined provisioner + node
 *
 * Initializes the mesh stack once in provisioner mode with a composition
 * that includes both provisioner and application models. Registers all
 * callbacks (provisioning, config client, config server, generic client,
 * generic server). Binds the local AppKey to every application model.
 *
 * @param cb Server event callback for incoming server messages
 * @return
 *   - ESP_OK on success
 *   - Error codes from BLE Mesh initialization functions
 *
 * @note Must be called after bluetooth_init() / pre_init()
 */
esp_err_t ble_mesh_provisioner_node_init(mesh_server_evt_cb_t cb);

#endif // DEZIBOT_BLUETOOTH_MESH_PROVISIONER_NODE_H
