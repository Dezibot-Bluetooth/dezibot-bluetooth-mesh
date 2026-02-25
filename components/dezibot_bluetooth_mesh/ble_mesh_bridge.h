/**
 * @file ble_mesh_bridge.h
 * @brief C ABI bridge for DeziBot Bluetooth Mesh.
 *
 * Provides a small, C-linkage wrapper around the underlying library so the mesh
 * stack can be initialized and used from C or other language bindings.
 */

#ifndef DEZIBOT_MESH_C_WRAPPER_H
#define DEZIBOT_MESH_C_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "esp_err.h"
#include <server/server_events.h>

/**
 * @brief Perform pre-initialization for the mesh stack.
 *
 * Thin C wrapper around the library's pre-init sequence (e.g. NVS + Bluetooth).
 *
 * @return
 *   - ESP_OK on success
 *   - ESP_FAIL / other esp_err_t on failure
 */
esp_err_t mesh_bridge_pre_init();

/**
 * @brief Initialize the Bluetooth host stack.
 *
 * @return
 *   - ESP_OK on success
 *   - ESP_FAIL / other esp_err_t on failure
 */
esp_err_t mesh_bridge_bluetooth_init();

/**
 * @brief Get the device UUID used for provisioning.
 *
 * @param[out] uuid Output buffer (16 bytes)
 */
void mesh_bridge_get_device_uuid(uint8_t *uuid);

/**
 * @brief Initialize the BLE Mesh client models.
 *
 * @return
 *   - ESP_OK on success
 *   - ESP_FAIL / other esp_err_t on failure
 */
esp_err_t mesh_bridge_client_init(char *device_name);

/**
 * @brief Send Generic OnOff Get.
 *
 * @param[in] addr Target unicast or group address.
 * @param[in] elem_index Local element index to use.
 *
 * @return
 *   - ESP_OK on success
 *   - ESP_FAIL / other esp_err_t on failure
 */
esp_err_t mesh_bridge_client_get_onoff(uint16_t addr, uint8_t elem_index);

/**
 * @brief Send Generic OnOff Set Unacknowledged.
 *
 * @param[in] val Target OnOff value.
 * @param[in] addr Target unicast or group address.
 * @param[in] elem_index Local element index to use.
 *
 * @return
 *   - ESP_OK on success
 *   - ESP_FAIL / other esp_err_t on failure
 */
esp_err_t mesh_bridge_client_set_onoff(uint8_t val, uint16_t addr, uint8_t elem_index);

/**
 * @brief Send Generic Level Get.
 *
 * @param[in] addr Target unicast or group address.
 * @param[in] elem_index Local element index to use.
 *
 * @return
 *   - ESP_OK on success
 *   - ESP_FAIL / other esp_err_t on failure
 */
esp_err_t mesh_bridge_client_get_level(uint16_t addr, uint8_t elem_index);

/**
 * @brief Send Generic Level Set Unacknowledged.
 *
 * @param[in] level Target level value.
 * @param[in] addr Target unicast or group address.
 * @param[in] elem_index Local element index to use.
 *
 * @return
 *   - ESP_OK on success
 *   - ESP_FAIL / other esp_err_t on failure
 */
esp_err_t mesh_bridge_client_set_level(int16_t level, uint16_t addr, uint8_t elem_index);

/**
 * @brief Send Generic Delta Set Unacknowledged.
 *
 * @param[in] delta Delta level value.
 * @param[in] addr Target unicast or group address.
 * @param[in] elem_index Local element index to use.
 *
 * @return
 *   - ESP_OK on success
 *   - ESP_FAIL / other esp_err_t on failure
 */
esp_err_t mesh_bridge_client_set_delta(int32_t delta, uint16_t addr, uint8_t elem_index);

/**
 * @brief Send Generic Move Set Unacknowledged.
 *
 * @param[in] move Move level value.
 * @param[in] addr Target unicast or group address.
 * @param[in] elem_index Local element index to use.
 *
 * @return
 *   - ESP_OK on success
 *   - ESP_FAIL / other esp_err_t on failure
 */
esp_err_t mesh_bridge_client_set_move(int16_t move, uint16_t addr, uint8_t elem_index);

/**
 * @brief Send Generic Default Transition Time Get.
 *
 * @param[in] addr Target unicast or group address.
 * @param[in] elem_index Local element index to use.
 *
 * @return
 *   - ESP_OK on success
 *   - ESP_FAIL / other esp_err_t on failure
 */
esp_err_t mesh_bridge_client_get_default_transition_time(uint16_t addr, uint8_t elem_index);

/**
 * @brief Send Generic Default Transition Time Set Unacknowledged.
 *
 * @param[in] transition_time Transition time value.
 * @param[in] addr Target unicast or group address.
 * @param[in] elem_index Local element index to use.
 *
 * @return
 *   - ESP_OK on success
 *   - ESP_FAIL / other esp_err_t on failure
 */
esp_err_t mesh_bridge_client_set_default_transition_time(uint8_t transition_time, uint16_t addr, uint8_t elem_index);

/**
 * @brief Send Generic OnPowerUp Get.
 *
 * @param[in] addr Target unicast or group address.
 * @param[in] elem_index Local element index to use.
 *
 * @return
 *   - ESP_OK on success
 *   - ESP_FAIL / other esp_err_t on failure
 */
esp_err_t mesh_bridge_client_get_onpowerup(uint16_t addr, uint8_t elem_index);

/**
 * @brief Send Generic OnPowerUp Set Unacknowledged.
 *
 * @param[in] power OnPowerUp value.
 * @param[in] addr Target unicast or group address.
 * @param[in] elem_index Local element index to use.
 *
 * @return
 *   - ESP_OK on success
 *   - ESP_FAIL / other esp_err_t on failure
 */
esp_err_t mesh_bridge_client_set_onpowerup(uint8_t power, uint16_t addr, uint8_t elem_index);

/**
 * @brief Send Generic Power Level Get.
 *
 * @param[in] addr Target unicast or group address.
 * @param[in] elem_index Local element index to use.
 *
 * @return
 *   - ESP_OK on success
 *   - ESP_FAIL / other esp_err_t on failure
 */
esp_err_t mesh_bridge_client_get_power_level(uint16_t addr, uint8_t elem_index);

/**
 * @brief Send Generic Power Level Set Unacknowledged.
 *
 * @param[in] power Target power value.
 * @param[in] addr Target unicast or group address.
 * @param[in] elem_index Local element index to use.
 *
 * @return
 *   - ESP_OK on success
 *   - ESP_FAIL / other esp_err_t on failure
 */
esp_err_t mesh_bridge_client_set_power_level(uint16_t power, uint16_t addr, uint8_t elem_index);

/**
 * @brief Send Generic Power Default Get.
 *
 * @param[in] addr Target unicast or group address.
 * @param[in] elem_index Local element index to use.
 *
 * @return
 *   - ESP_OK on success
 *   - ESP_FAIL / other esp_err_t on failure
 */
esp_err_t mesh_bridge_client_get_power_default(uint16_t addr, uint8_t elem_index);

/**
 * @brief Send Generic Power Default Set Unacknowledged.
 *
 * @param[in] power_default Target power default value.
 * @param[in] addr Target unicast or group address.
 * @param[in] elem_index Local element index to use.
 *
 * @return
 *   - ESP_OK on success
 *   - ESP_FAIL / other esp_err_t on failure
 */
esp_err_t mesh_bridge_client_set_power_default(uint16_t power_default, uint16_t addr, uint8_t elem_index);

/**
 * @brief Send Generic Power Range Get.
 *
 * @param[in] addr Target unicast or group address.
 * @param[in] elem_index Local element index to use.
 *
 * @return
 *   - ESP_OK on success
 *   - ESP_FAIL / other esp_err_t on failure
 */
esp_err_t mesh_bridge_client_get_power_range(uint16_t addr, uint8_t elem_index);

/**
 * @brief Send Generic Power Range Set Unacknowledged.
 *
 * @param[in] min Minimum power value.
 * @param[in] max Maximum power value.
 * @param[in] addr Target unicast or group address.
 * @param[in] elem_index Local element index to use.
 *
 * @return
 *   - ESP_OK on success
 *   - ESP_FAIL / other esp_err_t on failure
 */
esp_err_t mesh_bridge_client_set_power_range(uint16_t min, uint16_t max, uint16_t addr, uint8_t elem_index);

/**
 * @brief Send Generic Battery Get.
 *
 * @param[in] addr Target unicast or group address.
 * @param[in] elem_index Local element index to use.
 *
 * @return
 *   - ESP_OK on success
 *   - ESP_FAIL / other esp_err_t on failure
 */
esp_err_t mesh_bridge_client_get_battery(uint16_t addr, uint8_t elem_index);

/**
 * @brief Send Generic Global Location Get.
 *
 * @param[in] addr Target unicast or group address.
 * @param[in] elem_index Local element index to use.
 *
 * @return
 *   - ESP_OK on success
 *   - ESP_FAIL / other esp_err_t on failure
 */
esp_err_t mesh_bridge_client_get_loc_global(uint16_t addr, uint8_t elem_index);

/**
 * @brief Send Generic Global Location Set Unacknowledged.
 *
 * @param[in] latitude Global latitude value.
 * @param[in] longitude Global longitude value.
 * @param[in] altitude Global altitude value.
 * @param[in] addr Target unicast or group address.
 * @param[in] elem_index Local element index to use.
 *
 * @return
 *   - ESP_OK on success
 *   - ESP_FAIL / other esp_err_t on failure
 */
esp_err_t mesh_bridge_client_set_loc_global(int32_t latitude, int32_t longitude, int16_t altitude, uint16_t addr, uint8_t elem_index);

/**
 * @brief Send Generic Local Location Get.
 *
 * @param[in] addr Target unicast or group address.
 * @param[in] elem_index Local element index to use.
 *
 * @return
 *   - ESP_OK on success
 *   - ESP_FAIL / other esp_err_t on failure
 */
esp_err_t mesh_bridge_client_get_loc_local(uint16_t addr, uint8_t elem_index);

/**
 * @brief Send Generic Local Location Set Unacknowledged.
 *
 * @param[in] north Local north value.
 * @param[in] east Local east value.
 * @param[in] altitude Local altitude value.
 * @param[in] floor_number Local floor number.
 * @param[in] uncertainty Uncertainty value.
 * @param[in] addr Target unicast or group address.
 * @param[in] elem_index Local element index to use.
 *
 * @return
 *   - ESP_OK on success
 *   - ESP_FAIL / other esp_err_t on failure
 */
esp_err_t mesh_bridge_client_set_loc_local(int16_t north, int16_t east, int16_t altitude, uint8_t floor_number,
                                        uint16_t uncertainty, uint16_t addr, uint8_t elem_index);

/**
 * @brief Send Generic User Properties Get.
 *
 * @param[in] addr Target unicast or group address.
 * @param[in] elem_index Local element index to use.
 *
 * @return
 *   - ESP_OK on success
 *   - ESP_FAIL / other esp_err_t on failure
 */
esp_err_t mesh_bridge_client_get_user_properties(uint16_t addr, uint8_t elem_index);

/**
 * @brief Send Generic User Property Get.
 *
 * @param[in] property_id Property ID.
 * @param[in] addr Target unicast or group address.
 * @param[in] elem_index Local element index to use.
 *
 * @return
 *   - ESP_OK on success
 *   - ESP_FAIL / other esp_err_t on failure
 */
esp_err_t mesh_bridge_client_get_user_property(uint16_t property_id, uint16_t addr, uint8_t elem_index);

/**
 * @brief Send Generic User Property Set Unacknowledged.
 *
 * @param[in] property_id Property ID.
 * @param[in] property_value Pointer to property value buffer.
 * @param[in] property_value_len Length of property value buffer.
 * @param[in] addr Target unicast or group address.
 * @param[in] elem_index Local element index to use.
 *
 * @return
 *   - ESP_OK on success
 *   - ESP_FAIL / other esp_err_t on failure
 */
esp_err_t mesh_bridge_client_set_user_property(uint16_t property_id, uint8_t *property_value,
                                    uint16_t property_value_len, uint16_t addr, uint8_t elem_index);

/**
 * @brief Send Generic Admin Properties Get.
 *
 * @param[in] addr Target unicast or group address.
 * @param[in] elem_index Local element index to use.
 *
 * @return
 *   - ESP_OK on success
 *   - ESP_FAIL / other esp_err_t on failure
 */
esp_err_t mesh_bridge_client_get_admin_properties(uint16_t addr, uint8_t elem_index);

/**
 * @brief Send Generic Admin Property Get.
 *
 * @param[in] property_id Property ID.
 * @param[in] addr Target unicast or group address.
 * @param[in] elem_index Local element index to use.
 *
 * @return
 *   - ESP_OK on success
 *   - ESP_FAIL / other esp_err_t on failure
 */
esp_err_t mesh_bridge_client_get_admin_property(uint16_t property_id, uint16_t addr, uint8_t elem_index);

/**
 * @brief Send Generic Admin Property Set Unacknowledged.
 *
 * @param[in] property_id Property ID.
 * @param[in] admin_access Admin access value.
 * @param[in] property_value Pointer to property value buffer.
 * @param[in] property_value_len Length of property value buffer.
 * @param[in] addr Target unicast or group address.
 * @param[in] elem_index Local element index to use.
 *
 * @return
 *   - ESP_OK on success
 *   - ESP_FAIL / other esp_err_t on failure
 */
esp_err_t mesh_bridge_client_set_admin_property(uint16_t property_id, uint8_t admin_access, uint8_t *property_value,
                                    uint16_t property_value_len, uint16_t addr, uint8_t elem_index);

/**
 * @brief Send Generic Manufacturer Properties Get.
 *
 * @param[in] addr Target unicast or group address.
 * @param[in] elem_index Local element index to use.
 *
 * @return
 *   - ESP_OK on success
 *   - ESP_FAIL / other esp_err_t on failure
 */
esp_err_t mesh_bridge_client_get_manu_properties(uint16_t addr, uint8_t elem_index);

/**
 * @brief Send Generic Manufacturer Property Get.
 *
 * @param[in] property_id Property ID.
 * @param[in] addr Target unicast or group address.
 * @param[in] elem_index Local element index to use.
 *
 * @return
 *   - ESP_OK on success
 *   - ESP_FAIL / other esp_err_t on failure
 */
esp_err_t mesh_bridge_client_get_manu_property(uint16_t property_id, uint16_t addr, uint8_t elem_index);

/**
 * @brief Send Generic Manufacturer Property Set Unacknowledged.
 *
 * @param[in] property_id Property ID.
 * @param[in] admin_access Admin access value.
 * @param[in] addr Target unicast or group address.
 * @param[in] elem_index Local element index to use.
 *
 * @return
 *   - ESP_OK on success
 *   - ESP_FAIL / other esp_err_t on failure
 */
esp_err_t mesh_bridge_client_set_manu_property(uint16_t property_id, uint8_t admin_access, uint16_t addr, uint8_t elem_index);

/**
 * @brief Send Generic Client Properties Get.
 *
 * @param[in] property_id Client property ID.
 * @param[in] addr Target unicast or group address.
 * @param[in] elem_index Local element index to use.
 *
 * @return
 *   - ESP_OK on success
 *   - ESP_FAIL / other esp_err_t on failure
 */
esp_err_t mesh_bridge_client_get_client_properties(uint16_t property_id, uint16_t addr, uint8_t elem_index);

/**
 * @brief Initialize the BLE Mesh server models.
 *
 * @param[in] cb Application callback for incoming server events.
 *
 * @return
 *   - ESP_OK on success
 *   - ESP_FAIL / other esp_err_t on failure
 */
esp_err_t mesh_bridge_server_init(char *device_name, mesh_server_evt_cb_t cb);

/**
 * @brief Initialize BLE Mesh node (client + server).
 *
 * Initializes a node with both client and server models enabled and registers
 * the application callback for incoming server events.
 *
 * @param[in] cb Application callback for incoming server events.
 *
 * @return
 *   - ESP_OK on success
 *   - ESP_FAIL / other esp_err_t on failure
 */
esp_err_t mesh_bridge_node_init(char *device_name, mesh_server_evt_cb_t cb);

#ifdef __cplusplus
}
#endif

#endif // DEZIBOT_MESH_C_WRAPPER_H
