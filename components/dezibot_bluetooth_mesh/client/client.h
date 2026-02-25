#ifndef DEZIBOT_BLUETOOTH_MESH_CLIENT_H
#define DEZIBOT_BLUETOOTH_MESH_CLIENT_H

#include "../common/common.h"

void ble_mesh_client_set_composition(esp_ble_mesh_comp_t *comp);

esp_err_t ble_mesh_client_init(void);

// Generic OnOff Client
esp_err_t ble_mesh_client_get_onoff(uint16_t addr, uint8_t elem_index);

esp_err_t ble_mesh_client_set_onoff(uint8_t val, uint16_t addr, uint8_t elem_index);

// Generic Level Client
esp_err_t ble_mesh_client_get_level(uint16_t addr, uint8_t elem_index);

esp_err_t ble_mesh_client_set_level(int16_t level, uint16_t addr, uint8_t elem_index);

esp_err_t ble_mesh_client_set_delta(int32_t delta, uint16_t addr, uint8_t elem_index);

esp_err_t ble_mesh_client_set_move(int16_t move, uint16_t addr, uint8_t elem_index);

// Generic Default Transition Time Client
esp_err_t ble_mesh_client_get_default_transition_time(uint16_t addr, uint8_t elem_index);

esp_err_t ble_mesh_client_set_default_transition_time(uint8_t transition_time, uint16_t addr, uint8_t elem_index);

// Generic Power OnOff Client
esp_err_t ble_mesh_client_get_onpowerup(uint16_t addr, uint8_t elem_index);

esp_err_t ble_mesh_client_set_onpowerup(uint8_t power, uint16_t addr, uint8_t elem_index);

// Generic Power Level Client
esp_err_t ble_mesh_client_get_power_level(uint16_t addr, uint8_t elem_index);

esp_err_t ble_mesh_client_set_power_level(uint16_t power, uint16_t addr, uint8_t elem_index);

esp_err_t ble_mesh_client_get_power_default(uint16_t addr, uint8_t elem_index);

esp_err_t ble_mesh_client_set_power_default(uint16_t power_default, uint16_t addr, uint8_t elem_index);

esp_err_t ble_mesh_client_get_power_range(uint16_t addr, uint8_t elem_index);

esp_err_t ble_mesh_client_set_power_range(uint16_t min, uint16_t max, uint16_t addr, uint8_t elem_index);

// Generic Battery Client
esp_err_t ble_mesh_client_get_battery(uint16_t addr, uint8_t elem_index);

// Generic Location Client
esp_err_t ble_mesh_client_get_loc_global(uint16_t addr, uint8_t elem_index);

esp_err_t ble_mesh_client_set_loc_global(int32_t latitude, int32_t longitude, int16_t altitude, uint16_t addr, uint8_t elem_index);

esp_err_t ble_mesh_client_get_loc_local(uint16_t addr, uint8_t elem_index);

esp_err_t ble_mesh_client_set_loc_local(int16_t north, int16_t east, int16_t altitude, uint8_t floor_number,
                                        uint16_t uncertainty, uint16_t addr, uint8_t elem_index);

// Generic Property Client
esp_err_t ble_mesh_client_get_user_properties(uint16_t addr, uint8_t elem_index);

esp_err_t ble_mesh_client_get_user_property(uint16_t property_id, uint16_t addr, uint8_t elem_index);

esp_err_t ble_mesh_client_set_user_property(uint16_t property_id, uint8_t *property_value,
                                    uint16_t property_value_len, uint16_t addr, uint8_t elem_index);

esp_err_t ble_mesh_client_get_admin_properties(uint16_t addr, uint8_t elem_index);

esp_err_t ble_mesh_client_get_admin_property(uint16_t property_id, uint16_t addr, uint8_t elem_index);

esp_err_t ble_mesh_client_set_admin_property(uint16_t property_id, uint8_t admin_access, uint8_t *property_value,
                                    uint16_t property_value_len, uint16_t addr, uint8_t elem_index);

esp_err_t ble_mesh_client_get_manu_properties(uint16_t addr, uint8_t elem_index);

esp_err_t ble_mesh_client_get_manu_property(uint16_t property_id, uint16_t addr, uint8_t elem_index);

esp_err_t ble_mesh_client_set_manu_property(uint16_t property_id, uint8_t admin_access, uint16_t addr, uint8_t elem_index);

esp_err_t ble_mesh_client_get_client_properties(uint16_t property_id, uint16_t addr, uint8_t elem_index);

#endif //DEZIBOT_BLUETOOTH_MESH_CLIENT_H
