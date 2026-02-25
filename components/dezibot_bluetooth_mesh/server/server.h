#ifndef DEZIBOT_BLUETOOTH_MESH_SERVER_H
#define DEZIBOT_BLUETOOTH_MESH_SERVER_H

#include "common/common.h"
#include "server/server_events.h"

void ble_mesh_server_set_composition(esp_ble_mesh_comp_t *comp);

esp_err_t ble_mesh_server_init(mesh_server_evt_cb_t cb);

#endif //DEZIBOT_BLUETOOTH_MESH_SERVER_H
