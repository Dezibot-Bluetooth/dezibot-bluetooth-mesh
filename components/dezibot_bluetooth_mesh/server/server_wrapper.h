/**
 * @file server_wrapper.h
 * @brief Internal server glue (model state + event dispatch).
 */

#ifndef DEZIBOT_BLUETOOTH_MESH_SERVER_WRAPPER_H
#define DEZIBOT_BLUETOOTH_MESH_SERVER_WRAPPER_H

#include "server_events.h"
#include "esp_ble_mesh_defs.h"
#include "esp_ble_mesh_generic_model_api.h"

/**
 * @brief Container for a server model instance and its state.
 */
typedef struct {
    esp_ble_mesh_model_t *model;

    union {
        esp_ble_mesh_gen_onoff_srv_t onoff;
        esp_ble_mesh_gen_level_srv_t level;
        esp_ble_mesh_gen_level_srv_t delta;
        esp_ble_mesh_gen_level_srv_t move;
        esp_ble_mesh_gen_def_trans_time_srv_t def_trans_time;
        esp_ble_mesh_gen_power_onoff_srv_t onpowerup;
        esp_ble_mesh_gen_power_level_srv_t power_default;
        esp_ble_mesh_gen_power_level_srv_t power_level;
        esp_ble_mesh_gen_power_level_srv_t power_range;
        esp_ble_mesh_gen_battery_srv_t battery;
        esp_ble_mesh_gen_location_srv_t location;
        esp_ble_mesh_gen_client_prop_srv_t client_properties;
        esp_ble_mesh_gen_admin_prop_srv_t admin_prop;
        esp_ble_mesh_gen_manu_prop_srv_t manu_prop;
        esp_ble_mesh_gen_user_prop_srv_t user_prop;
    } srv;

    mesh_server_evt_cb_t cb;
} mesh_server_t;

/**
 * @brief Dispatch an event to the application's callback.
 *
 * @param[in] srv Server instance.
 * @param[in] evt Event to dispatch.
 */
void mesh_server_evt_dispatch(mesh_server_t *srv, const mesh_server_evt_t *evt);

#endif //DEZIBOT_BLUETOOTH_MESH_SERVER_WRAPPER_H
