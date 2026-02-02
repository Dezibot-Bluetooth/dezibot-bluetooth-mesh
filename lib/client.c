#include "client.h"
#include "bluetooth.h"
#include "common.h"

#define TAG "BLE_MESH_CLIENT"
#define APP_KEY_IDX 0x0000

static uint8_t dev_uuid[16];
static bool is_provisioned = false;
static uint16_t node_addr = 0;

static esp_ble_mesh_cfg_srv_t config_server = {
    .net_transmit = ESP_BLE_MESH_TRANSMIT(2, 20),
    .relay = ESP_BLE_MESH_RELAY_DISABLED,
    .relay_retransmit = ESP_BLE_MESH_TRANSMIT(2, 20),
    .beacon = ESP_BLE_MESH_BEACON_ENABLED,
    .gatt_proxy = ESP_BLE_MESH_GATT_PROXY_ENABLED,
    .friend_state = ESP_BLE_MESH_FRIEND_NOT_SUPPORTED,
    .default_ttl = 7
};

static esp_ble_mesh_client_t onoff_client;
static esp_ble_mesh_client_t config_client;
static esp_ble_mesh_model_pub_t pub;

static esp_ble_mesh_model_t client_models[] = {
    ESP_BLE_MESH_MODEL_CFG_SRV(&config_server),
    ESP_BLE_MESH_MODEL_CFG_CLI(&config_client),
    ESP_BLE_MESH_MODEL_GEN_ONOFF_CLI(&pub, &onoff_client),
};

static esp_ble_mesh_elem_t elements[] = {
    ESP_BLE_MESH_ELEMENT(0, client_models, ESP_BLE_MESH_MODEL_NONE),
};

static esp_ble_mesh_comp_t composition = {
    .cid = ESP_BLE_MESH_CID_NVAL,
    .elements = elements,
    .element_count = ARRAY_SIZE(elements),
};

static esp_ble_mesh_prov_t prov = {
    .uuid = dev_uuid,
};

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
            ESP_LOGW(TAG, "Steps: Tap 'Connect' on node -> Elements -> Element 0 -> Generic OnOff Client -> Bind Key");
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

static void mesh_config_client_cb(esp_ble_mesh_cfg_client_cb_event_t event,
                                  esp_ble_mesh_cfg_client_cb_param_t *param)
{
    esp_ble_mesh_client_common_param_t common = {0};
    esp_err_t err;

    switch (event) {
        case ESP_BLE_MESH_CFG_CLIENT_GET_STATE_EVT:
            ESP_LOGI(TAG, "Config client get state event");
            break;
        case ESP_BLE_MESH_CFG_CLIENT_SET_STATE_EVT:
            ESP_LOGI(TAG, "Config client set state event: opcode 0x%04x", param->params->opcode);
            if (param->params->opcode == ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND) {
                if (param->status_cb.model_app_status.status == 0) {
                    ESP_LOGI(TAG, "AppKey bound successfully to model 0x%04x", 
                             param->status_cb.model_app_status.model_id);
                } else {
                    ESP_LOGE(TAG, "AppKey bind failed with status 0x%02x", 
                             param->status_cb.model_app_status.status);
                }
            }
            break;
        case ESP_BLE_MESH_CFG_CLIENT_PUBLISH_EVT:
            ESP_LOGI(TAG, "Config client publish event");
            break;
        case ESP_BLE_MESH_CFG_CLIENT_TIMEOUT_EVT:
            ESP_LOGW(TAG, "Config client timeout, opcode 0x%04x", param->params->opcode);
            break;
        default:
            break;
    }
}

static void mesh_generic_client_cb(esp_ble_mesh_generic_client_cb_event_t event,
                                   esp_ble_mesh_generic_client_cb_param_t *param)
{
    switch (event) {
        case ESP_BLE_MESH_GENERIC_CLIENT_GET_STATE_EVT:
            ESP_LOGI(TAG, "Generic client get state event");
            break;
        case ESP_BLE_MESH_GENERIC_CLIENT_SET_STATE_EVT:
            ESP_LOGI(TAG, "Generic client set state event");
            break;
        case ESP_BLE_MESH_GENERIC_CLIENT_PUBLISH_EVT:
            ESP_LOGI(TAG, "Generic client publish event");
            break;
        case ESP_BLE_MESH_GENERIC_CLIENT_TIMEOUT_EVT:
            ESP_LOGI(TAG, "Generic client timeout event");
            break;
        default:
            break;
    }
}

void ble_mesh_client_send(uint8_t val, uint16_t addr)
{
    esp_ble_mesh_generic_client_set_state_t set_state = {0};
    esp_ble_mesh_client_common_param_t common = {0};
    esp_err_t err;
    
    if (!is_provisioned) {
        ESP_LOGW(TAG, "Device not provisioned yet, cannot send messages");
        return;
    }
    
    common.opcode = ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_SET_UNACK;
    common.model = &client_models[2];  // Generic OnOff Client is now at index 2
    common.ctx.net_idx = 0x0000;
    common.ctx.app_idx = 0x0000;
    common.ctx.addr = addr;
    common.ctx.send_ttl = 3;
    common.msg_timeout = 0;
    
    set_state.onoff_set.op_en = false;
    set_state.onoff_set.onoff = val;
    set_state.onoff_set.tid = 0;
    
    err = esp_ble_mesh_generic_client_set_state(&common, &set_state);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send onoff set (err %d)", err);
    } else {
        ESP_LOGI(TAG, "Sent onoff %d to addr 0x%04x", val, addr);
    }
}

esp_err_t ble_mesh_client_init(void)
{
    ESP_LOGI(TAG, "Initializing...");
    esp_err_t err;
    
    ble_mesh_get_dev_uuid(dev_uuid);
    ESP_LOGI(TAG, "Device UUID: %02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
             dev_uuid[0], dev_uuid[1], dev_uuid[2], dev_uuid[3],
             dev_uuid[4], dev_uuid[5], dev_uuid[6], dev_uuid[7],
             dev_uuid[8], dev_uuid[9], dev_uuid[10], dev_uuid[11],
             dev_uuid[12], dev_uuid[13], dev_uuid[14], dev_uuid[15]);
    
    err = esp_ble_mesh_register_prov_callback(mesh_prov_cb);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register prov callback (err %d)", err);
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
    
    err = esp_ble_mesh_init(&prov, &composition);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "BLE Mesh init failed (err %d)", err);
        return err;
    }
    
    err = esp_ble_mesh_node_prov_enable((esp_ble_mesh_prov_bearer_t)(ESP_BLE_MESH_PROV_ADV | ESP_BLE_MESH_PROV_GATT));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable node provisioning (err %d)", err);
        return err;
    }
    
    ESP_LOGI(TAG, "BLE Mesh Client initialized successfully");
    ESP_LOGI(TAG, "Device should now be visible for provisioning via GATT and ADV");
    
    return ESP_OK;
}
