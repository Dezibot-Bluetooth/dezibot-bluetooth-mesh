#include "ble_mesh_bridge.h"
#include "common/common.h"
#include "common/init.h"
#include "common/bluetooth.h"
#include "client/client.h"
#include "server/server.h"
#include "node/node.h"

// common/init
esp_err_t mesh_bridge_pre_init()
{
    return pre_init();
}

// common/bluetooth
void mesh_bridge_get_device_uuid(uint8_t *uuid)
{
    ble_mesh_get_dev_uuid(uuid);
}

esp_err_t mesh_bridge_bluetooth_init()
{
    return bluetooth_init();
}

// client/client
esp_err_t mesh_bridge_client_init(char *device_name)
{
    return ble_mesh_client_init(device_name);
}

esp_err_t mesh_bridge_client_get_onoff(uint16_t addr, uint8_t elem_index)
{
    return ble_mesh_client_get_onoff(addr, elem_index);
}

esp_err_t mesh_bridge_client_set_onoff(uint8_t val, uint16_t addr, uint8_t elem_index)
{
    return ble_mesh_client_set_onoff(val, addr, elem_index);
}

esp_err_t mesh_bridge_client_get_level(uint16_t addr, uint8_t elem_index)
{
    return ble_mesh_client_get_level(addr, elem_index);
}

esp_err_t mesh_bridge_client_set_level(int16_t level, uint16_t addr, uint8_t elem_index)
{
    return ble_mesh_client_set_level(level, addr, elem_index);
}

esp_err_t mesh_bridge_client_set_delta(int32_t delta, uint16_t addr, uint8_t elem_index)
{
    return ble_mesh_client_set_delta(delta, addr, elem_index);
}

esp_err_t mesh_bridge_client_set_move(int16_t move, uint16_t addr, uint8_t elem_index)
{
    return ble_mesh_client_set_move(move, addr, elem_index);
}

esp_err_t mesh_bridge_client_get_default_transition_time(uint16_t addr, uint8_t elem_index)
{
    return ble_mesh_client_get_default_transition_time(addr, elem_index);
}

esp_err_t mesh_bridge_client_set_default_transition_time(uint8_t transition_time, uint16_t addr, uint8_t elem_index)
{
    return ble_mesh_client_set_default_transition_time(transition_time, addr, elem_index);
}

esp_err_t mesh_bridge_client_get_onpowerup(uint16_t addr, uint8_t elem_index)
{
    return ble_mesh_client_get_onpowerup(addr, elem_index);
}

esp_err_t mesh_bridge_client_set_onpowerup(uint8_t power, uint16_t addr, uint8_t elem_index)
{
    return ble_mesh_client_set_onpowerup(power, addr, elem_index);
}

esp_err_t mesh_bridge_client_get_power_level(uint16_t addr, uint8_t elem_index)
{
    return ble_mesh_client_get_power_level(addr, elem_index);
}

esp_err_t mesh_bridge_client_set_power_level(uint16_t power, uint16_t addr, uint8_t elem_index)
{
    return ble_mesh_client_set_power_level(power, addr, elem_index);
}

esp_err_t mesh_bridge_client_get_power_default(uint16_t addr, uint8_t elem_index)
{
    return ble_mesh_client_get_power_default(addr, elem_index);
}

esp_err_t mesh_bridge_client_set_power_default(uint16_t power_default, uint16_t addr, uint8_t elem_index)
{
    return ble_mesh_client_set_power_default(power_default, addr, elem_index);
}

esp_err_t mesh_bridge_client_get_power_range(uint16_t addr, uint8_t elem_index)
{
    return ble_mesh_client_get_power_range(addr, elem_index);
}

esp_err_t mesh_bridge_client_set_power_range(uint16_t min, uint16_t max, uint16_t addr, uint8_t elem_index)
{
    return ble_mesh_client_set_power_range(min, max, addr, elem_index);
}

esp_err_t mesh_bridge_client_get_battery(uint16_t addr, uint8_t elem_index)
{
    return ble_mesh_client_get_battery(addr, elem_index);
}

esp_err_t mesh_bridge_client_get_loc_global(uint16_t addr, uint8_t elem_index)
{
    return ble_mesh_client_get_loc_global(addr, elem_index);
}

esp_err_t mesh_bridge_client_set_loc_global(int32_t latitude, int32_t longitude, int16_t altitude, uint16_t addr, uint8_t elem_index)
{
    return ble_mesh_client_set_loc_global(latitude, longitude, altitude, addr, elem_index);
}

esp_err_t mesh_bridge_client_get_loc_local(uint16_t addr, uint8_t elem_index)
{
    return ble_mesh_client_get_loc_local(addr, elem_index);
}

esp_err_t mesh_bridge_client_set_loc_local(int16_t north, int16_t east, int16_t altitude, uint8_t floor_number,
                                        uint16_t uncertainty, uint16_t addr, uint8_t elem_index)
{
    return ble_mesh_client_set_loc_local(north, east, altitude, floor_number, uncertainty, addr, elem_index);
}

esp_err_t mesh_bridge_client_get_user_properties(uint16_t addr, uint8_t elem_index)
{
    return ble_mesh_client_get_user_properties(addr, elem_index);
}

esp_err_t mesh_bridge_client_get_user_property(uint16_t property_id, uint16_t addr, uint8_t elem_index)
{
    return ble_mesh_client_get_user_property(property_id, addr, elem_index);
}

esp_err_t mesh_bridge_client_set_user_property(uint16_t property_id, uint8_t *property_value,
                                    uint16_t property_value_len, uint16_t addr, uint8_t elem_index)
{
    return ble_mesh_client_set_user_property(property_id, property_value, property_value_len, addr, elem_index);
}

esp_err_t mesh_bridge_client_get_admin_properties(uint16_t addr, uint8_t elem_index)
{
    return ble_mesh_client_get_admin_properties(addr, elem_index);
}

esp_err_t mesh_bridge_client_get_admin_property(uint16_t property_id, uint16_t addr, uint8_t elem_index)
{
    return ble_mesh_client_get_admin_property(property_id, addr, elem_index);
}

esp_err_t mesh_bridge_client_set_admin_property(uint16_t property_id, uint8_t admin_access, uint8_t *property_value,
                                    uint16_t property_value_len, uint16_t addr, uint8_t elem_index)
{
    return ble_mesh_client_set_admin_property(property_id, admin_access, property_value, property_value_len, addr, elem_index);
}

esp_err_t mesh_bridge_client_get_manu_properties(uint16_t addr, uint8_t elem_index)
{
    return ble_mesh_client_get_manu_properties(addr, elem_index);
}

esp_err_t mesh_bridge_client_get_manu_property(uint16_t property_id, uint16_t addr, uint8_t elem_index)
{
    return ble_mesh_client_get_manu_property(property_id, addr, elem_index);
}

esp_err_t mesh_bridge_client_set_manu_property(uint16_t property_id, uint8_t admin_access, uint16_t addr, uint8_t elem_index)
{
    return ble_mesh_client_set_manu_property(property_id, admin_access, addr, elem_index);
}

esp_err_t mesh_bridge_client_get_client_properties(uint16_t property_id, uint16_t addr, uint8_t elem_index)
{
    return ble_mesh_client_get_client_properties(property_id, addr, elem_index);
}

// server/server
esp_err_t mesh_bridge_server_init(char *device_name, mesh_server_evt_cb_t cb)
{
    return ble_mesh_server_init(device_name, cb);
}

// node/node
esp_err_t mesh_bridge_node_init(char *device_name, mesh_server_evt_cb_t cb)
{
    return ble_mesh_node_init(device_name, cb);
}
