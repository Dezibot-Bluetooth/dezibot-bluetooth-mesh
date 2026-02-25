#ifndef DEZIBOT_MESH_C_WRAPPER_H
#define DEZIBOT_MESH_C_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "esp_err.h"

esp_err_t mesh_bridge_pre_init(void);

esp_err_t mesh_bridge_bluetooth_init(void);

esp_err_t mesh_bridge_client_init(void);

void mesh_bridge_client_send(uint8_t val, uint16_t addr);

void mesh_bridge_get_device_uuid(uint8_t *uuid);

#ifdef __cplusplus
}
#endif

#endif // DEZIBOT_MESH_C_WRAPPER_H
