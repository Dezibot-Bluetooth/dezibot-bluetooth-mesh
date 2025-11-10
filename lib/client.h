#ifndef DEZIBOT_BLUETOOTH_MESH_CLIENT_H
#define DEZIBOT_BLUETOOTH_MESH_CLIENT_H

esp_err_t ble_mesh_client_init(void);

void ble_mesh_client_send(uint8_t val);

#endif //DEZIBOT_BLUETOOTH_MESH_CLIENT_H