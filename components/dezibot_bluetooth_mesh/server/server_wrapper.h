#ifndef DEZIBOT_BLUETOOTH_MESH_SERVER_WRAPPER_H
#define DEZIBOT_BLUETOOTH_MESH_SERVER_WRAPPER_H

#include "esp_ble_mesh_defs.h"
#include "esp_ble_mesh_generic_model_api.h"

typedef struct{
    /**
     * @note The srv union MUST be the first field so that
     *       &wrapper.srv.onoff == (void *)&wrapper.
     *       The ESP-IDF model macros store this address as
     *       model->user_data, and handle_gen_onoff_msg() casts
     *       it back to mesh_server_t*.
     */
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

    esp_ble_mesh_model_t *model;
    mesh_server_evt_cb_t cb;
} mesh_server_t;

void mesh_server_evt_dispatch(mesh_server_t *srv, const mesh_server_evt_t *evt);

#endif //DEZIBOT_BLUETOOTH_MESH_SERVER_WRAPPER_H