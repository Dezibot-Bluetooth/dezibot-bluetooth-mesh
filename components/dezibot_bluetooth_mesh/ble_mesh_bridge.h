#ifndef DEZIBOT_MESH_C_WRAPPER_H
#define DEZIBOT_MESH_C_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "esp_err.h"
#include <server/server_events.h>

// common/init
esp_err_t mesh_bridge_pre_init();

// common/bluetooth
esp_err_t mesh_bridge_bluetooth_init();

void mesh_bridge_get_device_uuid(uint8_t *uuid);

// client/client
esp_err_t mesh_bridge_client_init(char *device_name);

esp_err_t mesh_bridge_client_get_onoff(uint16_t addr, uint8_t elem_index);

esp_err_t mesh_bridge_client_set_onoff(uint8_t val, uint16_t addr, uint8_t elem_index);
    
esp_err_t mesh_bridge_client_get_level(uint16_t addr, uint8_t elem_index);

esp_err_t mesh_bridge_client_set_level(int16_t level, uint16_t addr, uint8_t elem_index);

esp_err_t mesh_bridge_client_set_delta(int32_t delta, uint16_t addr, uint8_t elem_index);

esp_err_t mesh_bridge_client_set_move(int16_t move, uint16_t addr, uint8_t elem_index);
    
esp_err_t mesh_bridge_client_get_default_transition_time(uint16_t addr, uint8_t elem_index);

esp_err_t mesh_bridge_client_set_default_transition_time(uint8_t transition_time, uint16_t addr, uint8_t elem_index);
    
esp_err_t mesh_bridge_client_get_onpowerup(uint16_t addr, uint8_t elem_index);

esp_err_t mesh_bridge_client_set_onpowerup(uint8_t power, uint16_t addr, uint8_t elem_index);
    
esp_err_t mesh_bridge_client_get_power_level(uint16_t addr, uint8_t elem_index);

esp_err_t mesh_bridge_client_set_power_level(uint16_t power, uint16_t addr, uint8_t elem_index);

esp_err_t mesh_bridge_client_get_power_default(uint16_t addr, uint8_t elem_index);

esp_err_t mesh_bridge_client_set_power_default(uint16_t power_default, uint16_t addr, uint8_t elem_index);

esp_err_t mesh_bridge_client_get_power_range(uint16_t addr, uint8_t elem_index);

esp_err_t mesh_bridge_client_set_power_range(uint16_t min, uint16_t max, uint16_t addr, uint8_t elem_index);
    
esp_err_t mesh_bridge_client_get_battery(uint16_t addr, uint8_t elem_index);
    
esp_err_t mesh_bridge_client_get_loc_global(uint16_t addr, uint8_t elem_index);

esp_err_t mesh_bridge_client_set_loc_global(int32_t latitude, int32_t longitude, int16_t altitude, uint16_t addr, uint8_t elem_index);

esp_err_t mesh_bridge_client_get_loc_local(uint16_t addr, uint8_t elem_index);

esp_err_t mesh_bridge_client_set_loc_local(int16_t north, int16_t east, int16_t altitude, uint8_t floor_number,
                                        uint16_t uncertainty, uint16_t addr, uint8_t elem_index);
    
esp_err_t mesh_bridge_client_get_user_properties(uint16_t addr, uint8_t elem_index);

esp_err_t mesh_bridge_client_get_user_property(uint16_t property_id, uint16_t addr, uint8_t elem_index);

esp_err_t mesh_bridge_client_set_user_property(uint16_t property_id, uint8_t *property_value,
                                    uint16_t property_value_len, uint16_t addr, uint8_t elem_index);

esp_err_t mesh_bridge_client_get_admin_properties(uint16_t addr, uint8_t elem_index);

esp_err_t mesh_bridge_client_get_admin_property(uint16_t property_id, uint16_t addr, uint8_t elem_index);

esp_err_t mesh_bridge_client_set_admin_property(uint16_t property_id, uint8_t admin_access, uint8_t *property_value,
                                    uint16_t property_value_len, uint16_t addr, uint8_t elem_index);

esp_err_t mesh_bridge_client_get_manu_properties(uint16_t addr, uint8_t elem_index);

esp_err_t mesh_bridge_client_get_manu_property(uint16_t property_id, uint16_t addr, uint8_t elem_index);

esp_err_t mesh_bridge_client_set_manu_property(uint16_t property_id, uint8_t admin_access, uint16_t addr, uint8_t elem_index);

esp_err_t mesh_bridge_client_get_client_properties(uint16_t property_id, uint16_t addr, uint8_t elem_index);

// server/server
esp_err_t mesh_bridge_server_init(char *device_name, mesh_server_evt_cb_t cb);

// node/node
esp_err_t mesh_bridge_node_init(char *device_name, mesh_server_evt_cb_t cb);

#ifdef __cplusplus
}
#endif

#endif // DEZIBOT_MESH_C_WRAPPER_H
