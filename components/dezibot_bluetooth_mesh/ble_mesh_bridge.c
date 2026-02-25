#include "ble_mesh_bridge.h"

#include "common/common.h"
#include "common/init.h"
#include "common/bluetooth.h"
#include "client/client.h"

esp_err_t mesh_bridge_pre_init(void)
{
    return pre_init();
}

esp_err_t mesh_bridge_bluetooth_init(void)
{
    return bluetooth_init();
}

esp_err_t mesh_bridge_client_init(void)
{
    return ble_mesh_client_init();
}

void mesh_bridge_client_send(uint8_t val, uint16_t addr)
{
    ble_mesh_client_send(val, addr);
}

void mesh_bridge_get_device_uuid(uint8_t *uuid)
{
    ble_mesh_get_dev_uuid(uuid);
}
