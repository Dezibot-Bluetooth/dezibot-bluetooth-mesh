#ifndef DEZIBOT_MESH_C_WRAPPER_H
#define DEZIBOT_MESH_C_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "esp_err.h"
#include <server/server_events.h>

// common/init
esp_err_t mesh_bridge_pre_init(void);

// common/bluetooth
esp_err_t mesh_bridge_bluetooth_init(void);

void mesh_bridge_get_device_uuid(uint8_t *uuid);

// client/client
esp_err_t mesh_bridge_client_init(void);

void mesh_bridge_client_send(uint8_t val, uint16_t addr);

// server/server
esp_err_t mesh_bridge_server_init(mesh_server_evt_cb_t cb);

#ifdef __cplusplus
}
#endif

#endif // DEZIBOT_MESH_C_WRAPPER_H
