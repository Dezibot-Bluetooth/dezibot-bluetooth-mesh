#ifndef DEZIBOT_BLUETOOTH_MESH_MESH_INIT_H
#define DEZIBOT_BLUETOOTH_MESH_MESH_INIT_H

#include "common.h"

void ble_mesh_get_dev_uuid(uint8_t *dev_uuid);

esp_err_t bluetooth_init(void);

#endif //DEZIBOT_BLUETOOTH_MESH_MESH_INIT_H