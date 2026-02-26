#include "node.h"

#include "client/client.h"
#include "common/common.h"
#include "common/bluetooth.h"
#include "server/server.h"
#include "server/server_events.h"
#include "server/server_wrapper.h"

#define TAG "BLE_MESH_NODE"
#define APP_KEY_IDX 0x0000

static uint8_t dev_uuid[16];
static bool is_provisioned = false;
static uint16_t node_addr = 0;

static esp_ble_mesh_comp_t *composition = NULL;

static esp_ble_mesh_prov_t prov = {
    .uuid = dev_uuid,
    .output_size = 0,
    .output_actions = 0
};

void ble_mesh_node_set_composition(esp_ble_mesh_comp_t *comp)
{
    composition = comp;
    ESP_LOGI(TAG, "Composition set: element_count=%u", composition->element_count);
    for (uint8_t i = 0; i < composition->element_count; i++)
    {
        esp_ble_mesh_model_t *mod = composition->elements[i].sig_models;
        for (uint8_t j = 0; j < composition->elements[i].sig_model_count; j++)
        {
            ESP_LOGI(TAG, "Model %u: id=0x%04x", j, mod[j].model_id);
        }
    }
}

static void mesh_prov_cb(esp_ble_mesh_prov_cb_event_t event,
                         esp_ble_mesh_prov_cb_param_t *param)
{
    switch (event) {
    case ESP_BLE_MESH_PROV_REGISTER_COMP_EVT:
        ESP_LOGI(TAG, "Provisioning stack initialized");
        break;
    case ESP_BLE_MESH_NODE_PROV_ENABLE_COMP_EVT:
        ESP_LOGI(TAG, "Node ready for provisioning - should be visible in nRF Mesh app");
        break;
    case ESP_BLE_MESH_NODE_PROV_LINK_OPEN_EVT:
        ESP_LOGI(TAG, "Provisioning link opened");
        break;
    case ESP_BLE_MESH_NODE_PROV_LINK_CLOSE_EVT:
        ESP_LOGI(TAG, "Provisioning link closed");
        break;
    case ESP_BLE_MESH_NODE_PROV_COMPLETE_EVT:
        ESP_LOGI(TAG, "Provisioning completed: addr=0x%04x", param->node_prov_complete.addr);
        node_addr = param->node_prov_complete.addr;
        is_provisioned = true;
        ESP_LOGI(TAG, "Device is now provisioned at address 0x%04x", node_addr);
        ESP_LOGI(TAG, "GATT Proxy should start advertising automatically");
        ESP_LOGW(TAG, "IMPORTANT: Reconnect to the device in nRF Mesh app, then bind the AppKey");
        ESP_LOGW(TAG, "Steps: Tap 'Connect' on node -> Elements -> Element 0 -> Generic OnOff Server -> Bind Key");
        break;
    case ESP_BLE_MESH_PROXY_CLIENT_RECV_ADV_PKT_EVT:
        ESP_LOGI(TAG, "Proxy client received advertising packet");
        break;
    case ESP_BLE_MESH_PROXY_CLIENT_CONNECTED_EVT:
        ESP_LOGI(TAG, "Proxy client connected");
        break;
    case ESP_BLE_MESH_PROXY_CLIENT_DISCONNECTED_EVT:
        ESP_LOGI(TAG, "Proxy client disconnected");
        break;
    default:
        ESP_LOGD(TAG, "Unhandled provisioning event %d", event);
        break;
    }
}

esp_err_t ble_mesh_node_init(char *device_name, mesh_server_evt_cb_t cb)
{
    ESP_LOGI(TAG, "Initializing...");

    if (composition->element_count == 0) {
        ESP_LOGE(TAG, "Composition data is empty. Please define at least one element with models.");
        return ESP_ERR_INVALID_STATE;
    }

    for (int i = 0; i < composition->element_count; ++i) {
        esp_ble_mesh_elem_t *elem = &composition->elements[i];
        if (!elem || !elem->sig_models) {
            continue;
        }
        for (int m = 0; m < elem->sig_model_count; ++m) {
            esp_ble_mesh_model_t *model = &elem->sig_models[m];

            if (model->model_id == ESP_BLE_MESH_MODEL_ID_GEN_ONOFF_CLI || model->model_id == ESP_BLE_MESH_MODEL_ID_GEN_LEVEL_CLI ||
                model->model_id == ESP_BLE_MESH_MODEL_ID_GEN_DEF_TRANS_TIME_CLI || model->model_id == ESP_BLE_MESH_MODEL_ID_GEN_POWER_ONOFF_CLI ||
                model->model_id == ESP_BLE_MESH_MODEL_ID_GEN_POWER_LEVEL_CLI || model->model_id == ESP_BLE_MESH_MODEL_ID_GEN_BATTERY_CLI ||
                model->model_id == ESP_BLE_MESH_MODEL_ID_GEN_LOCATION_CLI || model->model_id == ESP_BLE_MESH_MODEL_ID_GEN_PROP_CLI)
            {
                 continue;
            }

            if (model && model->user_data) {
                ((mesh_server_t *)model->user_data)->cb = cb;
            }
        }
    }

    ble_mesh_get_dev_uuid(dev_uuid);
    ESP_LOGI(TAG, "Device UUID: %02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
             dev_uuid[0], dev_uuid[1], dev_uuid[2], dev_uuid[3],
             dev_uuid[4], dev_uuid[5], dev_uuid[6], dev_uuid[7],
             dev_uuid[8], dev_uuid[9], dev_uuid[10], dev_uuid[11],
             dev_uuid[12], dev_uuid[13], dev_uuid[14], dev_uuid[15]);

    esp_err_t err;

    err = esp_ble_mesh_register_prov_callback(mesh_prov_cb);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register prov callback (err %d)", err);
        return err;
    }

    err = esp_ble_mesh_register_config_server_callback(mesh_config_server_cb);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register config server callback (err %d)", err);
        return err;
    }

    err = esp_ble_mesh_register_generic_server_callback(mesh_generic_server_cb);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register generic server callback (err %d)", err);
        return err;
    }

    err = esp_ble_mesh_register_config_client_callback(mesh_config_client_cb);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register config client callback (err %d)", err);
        return err;
    }

    err = esp_ble_mesh_register_generic_client_callback(mesh_generic_client_cb);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register generic client callback (err %d)", err);
        return err;
    }

    err = esp_ble_mesh_init(&prov, composition);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "BLE Mesh init failed (err %d)", err);
        return err;
    }

    err = esp_ble_mesh_set_unprovisioned_device_name(device_name);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to rename device(err %d)", err);
        return err;
    }

    err = esp_ble_mesh_node_prov_enable((esp_ble_mesh_prov_bearer_t)(ESP_BLE_MESH_PROV_ADV | ESP_BLE_MESH_PROV_GATT));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable node provisioning (err %d)", err);
        return err;
    }

    ESP_LOGI(TAG, "BLE Mesh Node initialized successfully");
    ESP_LOGI(TAG, "Device should now be visible for provisioning via GATT and ADV");

    return ESP_OK;
}
