#ifndef DEZIBOT_BLUETOOTH_MESH_PROVISIONER_H
#define DEZIBOT_BLUETOOTH_MESH_PROVISIONER_H

#include "common.h"

typedef struct {
    uint8_t  uuid[16];
    uint16_t unicast;
    uint8_t  elem_num;
} esp_ble_mesh_node_info_t;

typedef struct esp_ble_mesh_key {
    uint16_t net_idx;
    uint16_t app_idx;
    uint8_t  app_key[16];
} esp_ble_mesh_prov_key_t;

static esp_err_t ble_mesh_init(void);

#endif //DEZIBOT_BLUETOOTH_MESH_PROVISIONER_H