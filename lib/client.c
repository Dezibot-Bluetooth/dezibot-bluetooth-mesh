#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_ble_mesh_common_api.h"
#include "esp_ble_mesh_networking_api.h"
#include "esp_ble_mesh_provisioning_api.h"
#include "esp_ble_mesh_config_model_api.h"
#include "client.h"

#define TAG "BLE_MESH_CLIENT"

// static uint8_t dev_uuid[16] = { 0xdd, 0xdd };
static uint16_t server_addr = 0x0001; // ???

// static esp_ble_mesh_client_t client;
// static esp_ble_mesh_model_pub_t pub;

// static esp_ble_mesh_model_t client_models[] = {
//     ESP_BLE_MESH_MODEL_CFG_SRV(),
//     ESP_BLE_MESH_MODEL_GEN_CLI(&pub, &client),
// };
//
// static esp_ble_mesh_elem_t elements[] = {
//     ESP_BLE_MESH_ELEMENT(0, client_models, ESP_BLE_MESH_MODEL_NONE),
// };

// static esp_ble_mesh_comp_t composition = {
//     .cid = ESP_BLE_MESH_CID_ESP,
//     .elements = elements,
//     .element_count = ARRAY_SIZE(elements),
// };

//static esp_ble_mesh_prov_t prov = {
//    .uuid = dev_uuid,
//};

// static void mesh_prov_cb(esp_ble_mesh_prov_cb_event_t event,
//                          esp_ble_mesh_prov_cb_param_t *param)
// {
//     switch (event) {
//         case ESP_BLE_MESH_PROV_REGISTER_COMP_EVT:
//             ESP_LOGI(TAG, "Provisioning stack initialized");
//             break;
//         case ESP_BLE_MESH_NODE_PROV_ENABLE_COMP_EVT:
//             ESP_LOGI(TAG, "Node ready for provisioning");
//             break;
//         case ESP_BLE_MESH_NODE_PROV_LINK_OPEN_EVT:
//             ESP_LOGI(TAG, "Provisioning link opened");
//             break;
//         case ESP_BLE_MESH_NODE_PROV_LINK_CLOSE_EVT:
//             ESP_LOGI(TAG, "Provisioning link closed");
//             break;
//         case ESP_BLE_MESH_NODE_PROV_COMPLETE_EVT:
//             ESP_LOGI(TAG, "Provisioning completed: addr=0x%04x", param->node_prov_complete.addr);
//             break;
//         default:
//             ESP_LOGI(TAG, "Unhandled provisioning event %d", event);
//             break;
//     }
// }

// static void mesh_model_cb(esp_ble_mesh_model_cb_event_t event,
//                          esp_ble_mesh_model_cb_param_t *param)
// {
//     switch (event) {
//         case ESP_BLE_MESH_MODEL_OPERATION_EVT:
//             ESP_LOGI(TAG, "Model operation event");
//             break;
//         default:
//             break;
//     }
// }

void ble_mesh_client_send(const uint8_t val)
{
    ESP_LOGI(TAG, "Sent val: %dto server 0x%04x", val, server_addr);
}

esp_err_t ble_mesh_client_init(void)
{
    ESP_LOGI(TAG, "Initializing...");
    return ESP_OK;
}