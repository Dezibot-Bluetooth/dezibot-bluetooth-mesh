#ifndef DEZIBOT_BLUETOOTH_MESH_NODE_H
#define DEZIBOT_BLUETOOTH_MESH_NODE_H

#include "common.h"

/**
 * @brief Initialize BLE Mesh node with both client and server models
 * 
 * This initializes a complete mesh node that can:
 * - Send OnOff commands to other nodes (client functionality)
 * - Receive OnOff commands from other nodes (server functionality)
 * - Relay messages between nodes (mesh routing)
 * 
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t ble_mesh_node_init(void);

/**
 * @brief Send Generic OnOff command to another node (Client functionality)
 * @param onoff 0=OFF, 1=ON
 * @param addr Target unicast or group address
 */
void ble_mesh_node_send_onoff(uint8_t onoff, uint16_t addr);

/**
 * @brief Get current OnOff state (Server functionality)
 * @return Current OnOff state (0=OFF, 1=ON)
 */
uint8_t ble_mesh_node_get_onoff(void);

/**
 * @brief Manually set OnOff state (Server functionality)
 * @param onoff 0=OFF, 1=ON
 */
void ble_mesh_node_set_onoff(uint8_t onoff);

/**
 * @brief Check if node has been provisioned
 * @return true if provisioned, false otherwise
 */
bool ble_mesh_node_is_provisioned(void);

/**
 * @brief Get this node's unicast address
 * @return Unicast address (0x0000 if not provisioned)
 */
uint16_t ble_mesh_node_get_addr(void);

#endif //DEZIBOT_BLUETOOTH_MESH_NODE_H
