/**
 * @file provisioner.h
 * @brief Bluetooth Mesh provisioner implementation
 * 
 * This module implements a BLE Mesh provisioner - a special node responsible
 * for adding new devices to the mesh network. The provisioner authenticates,
 * configures, and assigns addresses to unprovisioned devices.
 * 
 * Key Responsibilities:
 * - Discover unprovisioned devices via advertising
 * - Provision devices into the mesh network
 * - Distribute network and application keys
 * - Configure node capabilities (composition data)
 * - Bind application keys to specific models
 * 
 * Automatic Configuration:
 * When a device is provisioned, the provisioner automatically:
 * 1. Retrieves composition data
 * 2. Adds application key to the node
 * 3. Binds Generic OnOff Server model
 * 
 * @author DeziBot Project
 * @date 2026
 */

#ifndef DEZIBOT_BLUETOOTH_MESH_PROVISIONER_H
#define DEZIBOT_BLUETOOTH_MESH_PROVISIONER_H

#include "../common/common.h"

/**
 * @brief Node information structure
 * 
 * Stores information about a provisioned node for tracking
 * and addressing purposes.
 */
typedef struct {
    uint8_t  uuid[16];      /**< Device unique identifier (16 bytes) */
    uint16_t unicast;       /**< Assigned unicast address */
    uint8_t  elem_num;      /**< Number of elements in the node */
} esp_ble_mesh_node_info_t;

/**
 * @brief Provisioning key structure
 * 
 * Stores cryptographic keys used for secure mesh communication.
 */
typedef struct esp_ble_mesh_key {
    uint16_t net_idx;       /**< Network key index */
    uint16_t app_idx;       /**< Application key index */
    uint8_t  app_key[16];   /**< Application key value (16 bytes) */
} esp_ble_mesh_prov_key_t;

/**
 * @brief Initialize BLE Mesh as provisioner
 * 
 * Initializes the BLE Mesh stack in provisioner mode. The provisioner
 * will automatically discover and provision devices with matching UUIDs.
 * 
 * Initialization steps:
 * 1. Configure provisioning keys (network and application)
 * 2. Register provisioning and configuration callbacks
 * 3. Initialize BLE Mesh stack with provisioner composition
 * 4. Set UUID matching filter (0xdd, 0xdd prefix)
 * 5. Enable provisioning on ADV and GATT bearers
 * 6. Add local application key
 * 
 * Configuration:
 * - Provisioner address: 0x0001
 * - Start address for nodes: 0x0005
 * - Application key: All bytes set to 0x12
 * - UUID filter: Devices starting with 0xdd 0xdd
 * 
 * @return 
 *   - ESP_OK: Provisioner initialized successfully
 *   - Error codes from BLE Mesh initialization functions
 * 
 * @note This function should be called after bluetooth_init()
 * @note The provisioner runs automatically once initialized
 * 
 * @see bluetooth_init()
 */
esp_err_t ble_mesh_init(void);

#endif //DEZIBOT_BLUETOOTH_MESH_PROVISIONER_H