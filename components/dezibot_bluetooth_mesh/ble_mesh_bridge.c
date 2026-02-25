#include "ble_mesh_bridge.h"

#include "common/common.h"
#include "common/init.h"
#include "common/bluetooth.h"
#include "client/client.h"
#include "server/server.h"

// common/init
esp_err_t mesh_bridge_pre_init(void)
{
    return pre_init();
}

// common/bluetooth
void mesh_bridge_get_device_uuid(uint8_t *uuid)
{
    ble_mesh_get_dev_uuid(uuid);
}

esp_err_t mesh_bridge_bluetooth_init(void)
{
    return bluetooth_init();
}

// client/client
esp_err_t mesh_bridge_client_init(void)
{
    return ble_mesh_client_init();
}

void mesh_bridge_client_send(uint8_t val, uint16_t addr)
{
    ble_mesh_client_send(val, addr);
}

// server/server
esp_err_t mesh_bridge_server_init(mesh_server_evt_cb_t cb)
{
    return ble_mesh_server_init(cb);
}
