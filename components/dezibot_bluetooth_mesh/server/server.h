#ifndef DEZIBOT_BLUETOOTH_MESH_SERVER_H
#define DEZIBOT_BLUETOOTH_MESH_SERVER_H

#include "common.h"

esp_err_t ble_mesh_server_init(void);

// Generic OnOff Server - get current on/off state
uint8_t ble_mesh_server_get_onoff(void);

// Generic OnOff Server - set on/off state
void ble_mesh_server_set_onoff(uint8_t onoff);

#endif //DEZIBOT_BLUETOOTH_MESH_SERVER_H
