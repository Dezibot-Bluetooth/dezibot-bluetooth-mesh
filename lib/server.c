#include "server.h"
#include "bluetooth.h"
#include "common.h"

#define TAG "BLE_MESH_SERVER"
#define APP_KEY_IDX 0x0000

static uint8_t dev_uuid[16];
static bool is_provisioned = false;
static uint16_t node_addr = 0;
static uint8_t onoff_state = 0;

static esp_ble_mesh_cfg_srv_t config_server = {
    .net_transmit = ESP_BLE_MESH_TRANSMIT(2, 20),
    .relay = ESP_BLE_MESH_RELAY_DISABLED,
    .relay_retransmit = ESP_BLE_MESH_TRANSMIT(2, 20),
    .beacon = ESP_BLE_MESH_BEACON_ENABLED,
    .gatt_proxy = ESP_BLE_MESH_GATT_PROXY_ENABLED,
    .friend_state = ESP_BLE_MESH_FRIEND_NOT_SUPPORTED,
    .default_ttl = 7
};

static esp_ble_mesh_gen_onoff_srv_t onoff_server;
static esp_ble_mesh_model_pub_t pub;

static esp_ble_mesh_model_t server_models[] = {
    ESP_BLE_MESH_MODEL_CFG_SRV(&config_server),
    ESP_BLE_MESH_MODEL_GEN_ONOFF_SRV(&pub, &onoff_server),
};

static esp_ble_mesh_elem_t elements[] = {
    ESP_BLE_MESH_ELEMENT(0, server_models, ESP_BLE_MESH_MODEL_NONE),
};

static esp_ble_mesh_comp_t composition = {
    .cid = ESP_BLE_MESH_CID_NVAL,
    .element_count = ARRAY_SIZE(elements),
    .elements = elements,
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

static void mesh_config_server_cb(esp_ble_mesh_cfg_server_cb_event_t event,
                                  esp_ble_mesh_cfg_server_cb_param_t *param)
{
    switch (event) {
        case ESP_BLE_MESH_CFG_SERVER_STATE_CHANGE_EVT:
            ESP_LOGI(TAG, "Config server state changed");
            break;
        default:
            break;
    }
}

static void mesh_generic_server_cb(esp_ble_mesh_generic_server_cb_event_t event,
                                   esp_ble_mesh_generic_server_cb_param_t *param)
{
    esp_ble_mesh_generic_server_cb_param_t *p = param;

    switch (event) {
        case ESP_BLE_MESH_GENERIC_SERVER_STATE_CHANGE_EVT:
            ESP_LOGI(TAG, "Generic server state changed");
            if (p->ctx.recv_op == ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_SET) {
                onoff_state = p->value.state_change.onoff_set.onoff;
                ESP_LOGI(TAG, "OnOff state changed to %d", onoff_state);
            }
            break;
        case ESP_BLE_MESH_GENERIC_SERVER_RECV_GET_MSG_EVT:
            ESP_LOGI(TAG, "Generic server received get message, opcode 0x%04x", p->ctx.recv_op);
            break;
        case ESP_BLE_MESH_GENERIC_SERVER_RECV_SET_MSG_EVT:
            ESP_LOGI(TAG, "Generic server received set message, opcode 0x%04x", p->ctx.recv_op);
            if (p->ctx.recv_op == ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_SET ||
                p->ctx.recv_op == ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_SET_UNACK) {
                onoff_state = p->value.set.onoff.onoff;
                ESP_LOGI(TAG, "Received OnOff command: set to %d (TID: %d)", 
                         onoff_state, p->value.set.onoff.tid);
            }
            break;
        default:
            break;
    }
}

uint8_t ble_mesh_server_get_onoff(void)
{
    return onoff_state;
}

void ble_mesh_server_set_onoff(uint8_t onoff)
{
    onoff_state = onoff;
    ESP_LOGI(TAG, "OnOff state set to %d", onoff_state);
}

esp_err_t ble_mesh_server_init(void)
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

    ESP_LOGI(TAG, "BLE Mesh Server initialized successfully");
    ESP_LOGI(TAG, "Device should now be visible for provisioning via GATT and ADV");

    return ESP_OK;
}
