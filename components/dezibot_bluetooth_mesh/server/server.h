#ifndef DEZIBOT_BLUETOOTH_MESH_SERVER_H
#define DEZIBOT_BLUETOOTH_MESH_SERVER_H

#include "common/common.h"
#include "server/server_events.h"

void ble_mesh_server_set_composition(esp_ble_mesh_comp_t *comp);

void mesh_generic_server_cb(esp_ble_mesh_generic_server_cb_event_t event, esp_ble_mesh_generic_server_cb_param_t *param);

void mesh_config_server_cb(esp_ble_mesh_cfg_server_cb_event_t event, esp_ble_mesh_cfg_server_cb_param_t *param);

esp_err_t ble_mesh_server_init(mesh_server_evt_cb_t cb);

#endif //DEZIBOT_BLUETOOTH_MESH_SERVER_H
