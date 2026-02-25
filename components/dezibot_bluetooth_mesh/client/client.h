/**
 * @file client.h
 * @brief BLE Mesh client API.
 */

#ifndef DEZIBOT_BLUETOOTH_MESH_CLIENT_H
#define DEZIBOT_BLUETOOTH_MESH_CLIENT_H

#include "../common/common.h"

/**
 * @brief BLE Mesh Configuration Client callback handler.
 *
 * Internal callback passed to the ESP BLE Mesh stack for Configuration Client
 * model events.
 *
 * @param[in] event Event ID from the ESP BLE Mesh stack.
 * @param[in] param Event parameters provided by the ESP BLE Mesh stack.
 */
void mesh_config_client_cb(esp_ble_mesh_cfg_client_cb_event_t event, esp_ble_mesh_cfg_client_cb_param_t *param);

/**
 * @brief BLE Mesh Generic Client callback handler.
 *
 * Internal callback passed to the ESP BLE Mesh stack for Generic Client model
 * events.
 *
 * @param[in] event Event ID from the ESP BLE Mesh stack.
 * @param[in] param Event parameters provided by the ESP BLE Mesh stack.
 */
void mesh_generic_client_cb(esp_ble_mesh_generic_client_cb_event_t event, esp_ble_mesh_generic_client_cb_param_t *param);

/**
 * @brief Override the default composition data.
 *
 * Call this before @ref ble_mesh_client_init if you want to provide a custom
 * element/model composition.
 *
 * @param[in] comp Composition data to use.
 */
void ble_mesh_client_set_composition(esp_ble_mesh_comp_t *comp);

/**
 * @brief Initialize BLE Mesh client models.
 *
 * Registers callbacks and initializes the client-side models.
 *
 * @return
 *   - ESP_OK on success
 *   - ESP_FAIL / other esp_err_t on failure
 */
esp_err_t ble_mesh_client_init(char *device_name);

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
esp_err_t ble_mesh_client_get_onoff(uint16_t addr, uint8_t elem_index);

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
esp_err_t ble_mesh_client_set_onoff(uint8_t val, uint16_t addr, uint8_t elem_index);

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
esp_err_t ble_mesh_client_get_level(uint16_t addr, uint8_t elem_index);

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
esp_err_t ble_mesh_client_set_level(int16_t level, uint16_t addr, uint8_t elem_index);

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
esp_err_t ble_mesh_client_set_delta(int32_t delta, uint16_t addr, uint8_t elem_index);

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
esp_err_t ble_mesh_client_set_move(int16_t move, uint16_t addr, uint8_t elem_index);

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
esp_err_t ble_mesh_client_get_default_transition_time(uint16_t addr, uint8_t elem_index);

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
esp_err_t ble_mesh_client_set_default_transition_time(uint8_t transition_time, uint16_t addr, uint8_t elem_index);

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
esp_err_t ble_mesh_client_get_onpowerup(uint16_t addr, uint8_t elem_index);

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
esp_err_t ble_mesh_client_set_onpowerup(uint8_t power, uint16_t addr, uint8_t elem_index);

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
esp_err_t ble_mesh_client_get_power_level(uint16_t addr, uint8_t elem_index);

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
esp_err_t ble_mesh_client_set_power_level(uint16_t power, uint16_t addr, uint8_t elem_index);

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
esp_err_t ble_mesh_client_get_power_default(uint16_t addr, uint8_t elem_index);

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
esp_err_t ble_mesh_client_set_power_default(uint16_t power_default, uint16_t addr, uint8_t elem_index);

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
esp_err_t ble_mesh_client_get_power_range(uint16_t addr, uint8_t elem_index);

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
esp_err_t ble_mesh_client_set_power_range(uint16_t min, uint16_t max, uint16_t addr, uint8_t elem_index);

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
esp_err_t ble_mesh_client_get_battery(uint16_t addr, uint8_t elem_index);

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
esp_err_t ble_mesh_client_get_loc_global(uint16_t addr, uint8_t elem_index);

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
esp_err_t ble_mesh_client_set_loc_global(int32_t latitude, int32_t longitude, int16_t altitude, uint16_t addr, uint8_t elem_index);

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
esp_err_t ble_mesh_client_get_loc_local(uint16_t addr, uint8_t elem_index);

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
esp_err_t ble_mesh_client_set_loc_local(int16_t north, int16_t east, int16_t altitude, uint8_t floor_number,
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
esp_err_t ble_mesh_client_get_user_properties(uint16_t addr, uint8_t elem_index);

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
esp_err_t ble_mesh_client_get_user_property(uint16_t property_id, uint16_t addr, uint8_t elem_index);

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
esp_err_t ble_mesh_client_set_user_property(uint16_t property_id, uint8_t *property_value,
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
esp_err_t ble_mesh_client_get_admin_properties(uint16_t addr, uint8_t elem_index);

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
esp_err_t ble_mesh_client_get_admin_property(uint16_t property_id, uint16_t addr, uint8_t elem_index);

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
esp_err_t ble_mesh_client_set_admin_property(uint16_t property_id, uint8_t admin_access, uint8_t *property_value,
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
esp_err_t ble_mesh_client_get_manu_properties(uint16_t addr, uint8_t elem_index);

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
esp_err_t ble_mesh_client_get_manu_property(uint16_t property_id, uint16_t addr, uint8_t elem_index);

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
esp_err_t ble_mesh_client_set_manu_property(uint16_t property_id, uint8_t admin_access, uint16_t addr, uint8_t elem_index);

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
esp_err_t ble_mesh_client_get_client_properties(uint16_t property_id, uint16_t addr, uint8_t elem_index);

#endif //DEZIBOT_BLUETOOTH_MESH_CLIENT_H
