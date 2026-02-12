#include "mesh_node.h"
#include "bluetooth.h"
#include "common.h"

#define TAG "BLE_MESH_NODE"
#define APP_KEY_IDX 0x0000

static uint8_t dev_uuid[16];
static bool is_provisioned = false;
static uint16_t node_addr = 0;
static uint8_t onoff_state = 0;

// Configuration Server - required for all nodes
static esp_ble_mesh_cfg_srv_t config_server = {
    .net_transmit = ESP_BLE_MESH_TRANSMIT(2, 20),
    .relay = ESP_BLE_MESH_RELAY_ENABLED,  // Enable relay for true mesh functionality
    .relay_retransmit = ESP_BLE_MESH_TRANSMIT(2, 20),
    .beacon = ESP_BLE_MESH_BEACON_ENABLED,
    .gatt_proxy = ESP_BLE_MESH_GATT_PROXY_ENABLED,
    .friend_state = ESP_BLE_MESH_FRIEND_NOT_SUPPORTED,
    .default_ttl = 7
};

// Client models - for sending commands
static esp_ble_mesh_client_t onoff_client;
static esp_ble_mesh_client_t config_client;

// Server models - for receiving commands
static esp_ble_mesh_gen_onoff_srv_t onoff_server = {
    .rsp_ctrl.get_auto_rsp = ESP_BLE_MESH_SERVER_AUTO_RSP,
    .rsp_ctrl.set_auto_rsp = ESP_BLE_MESH_SERVER_AUTO_RSP,
};

// Publication context (shared between models)
static esp_ble_mesh_model_pub_t onoff_cli_pub;
static esp_ble_mesh_model_pub_t onoff_srv_pub;

// Models array - contains BOTH client and server models
static esp_ble_mesh_model_t models[] = {
    ESP_BLE_MESH_MODEL_CFG_SRV(&config_server),
    ESP_BLE_MESH_MODEL_CFG_CLI(&config_client),
    ESP_BLE_MESH_MODEL_GEN_ONOFF_CLI(&onoff_cli_pub, &onoff_client),
    ESP_BLE_MESH_MODEL_GEN_ONOFF_SRV(&onoff_srv_pub, &onoff_server),
};

// Elements - one element containing all models
static esp_ble_mesh_elem_t elements[] = {
    ESP_BLE_MESH_ELEMENT(0, models, ESP_BLE_MESH_MODEL_NONE),
};

// Composition - describes the node's capabilities
static esp_ble_mesh_comp_t composition = {
    .cid = ESP_BLE_MESH_CID_NVAL,
    .elements = elements,
    .element_count = ARRAY_SIZE(elements),
};

// Provisioning data
static esp_ble_mesh_prov_t prov = {
    .uuid = dev_uuid,
};

/**
 * @brief Provisioning callback - handles provisioning events
 */
static void mesh_prov_cb(esp_ble_mesh_prov_cb_event_t event,
                         esp_ble_mesh_prov_cb_param_t *param)
{
    switch (event) {
        case ESP_BLE_MESH_PROV_REGISTER_COMP_EVT:
            ESP_LOGI(TAG, "Provisioning stack initialized");
            break;
        case ESP_BLE_MESH_NODE_PROV_ENABLE_COMP_EVT:
            ESP_LOGI(TAG, "Node ready for provisioning");
            ESP_LOGI(TAG, "This node has BOTH client and server models");
            break;
        case ESP_BLE_MESH_NODE_PROV_LINK_OPEN_EVT:
            ESP_LOGI(TAG, "Provisioning link opened");
            break;
        case ESP_BLE_MESH_NODE_PROV_LINK_CLOSE_EVT:
            ESP_LOGI(TAG, "Provisioning link closed");
            break;
        case ESP_BLE_MESH_NODE_PROV_COMPLETE_EVT:
            node_addr = param->node_prov_complete.addr;
            is_provisioned = true;
            ESP_LOGI(TAG, "Provisioning completed!");
            ESP_LOGI(TAG, "  Unicast address: 0x%04x", node_addr);
            ESP_LOGI(TAG, "  Can now SEND commands (client) and RECEIVE commands (server)");
            ESP_LOGI(TAG, "  Relay is ENABLED - will forward messages for other nodes");
            break;
        case ESP_BLE_MESH_PROXY_CLIENT_RECV_ADV_PKT_EVT:
            ESP_LOGD(TAG, "Proxy client received advertising packet");
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

/**
 * @brief Config server callback
 */
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

/**
 * @brief Config client callback
 */
static void mesh_config_client_cb(esp_ble_mesh_cfg_client_cb_event_t event,
                                  esp_ble_mesh_cfg_client_cb_param_t *param)
{
    switch (event) {
        case ESP_BLE_MESH_CFG_CLIENT_GET_STATE_EVT:
            ESP_LOGD(TAG, "Config client get state event");
            break;
        case ESP_BLE_MESH_CFG_CLIENT_SET_STATE_EVT:
            ESP_LOGI(TAG, "Config client set state event: opcode 0x%04x", param->params->opcode);
            break;
        case ESP_BLE_MESH_CFG_CLIENT_PUBLISH_EVT:
            ESP_LOGD(TAG, "Config client publish event");
            break;
        case ESP_BLE_MESH_CFG_CLIENT_TIMEOUT_EVT:
            ESP_LOGW(TAG, "Config client timeout, opcode 0x%04x", param->params->opcode);
            break;
        default:
            break;
    }
}

/**
 * @brief Generic client callback - handles responses from servers
 */
static void mesh_generic_client_cb(esp_ble_mesh_generic_client_cb_event_t event,
                                   esp_ble_mesh_generic_client_cb_param_t *param)
{
    switch (event) {
        case ESP_BLE_MESH_GENERIC_CLIENT_GET_STATE_EVT:
            ESP_LOGI(TAG, "Client: Received GET response");
            break;
        case ESP_BLE_MESH_GENERIC_CLIENT_SET_STATE_EVT:
            ESP_LOGI(TAG, "Client: Received SET response");
            break;
        case ESP_BLE_MESH_GENERIC_CLIENT_PUBLISH_EVT:
            ESP_LOGI(TAG, "Client: Publish event");
            break;
        case ESP_BLE_MESH_GENERIC_CLIENT_TIMEOUT_EVT:
            ESP_LOGD(TAG, "Client: Timeout (normal for unacknowledged messages)");
            break;
        default:
            break;
    }
}

/**
 * @brief Generic server callback - handles incoming commands from clients
 */
static void mesh_generic_server_cb(esp_ble_mesh_generic_server_cb_event_t event,
                                   esp_ble_mesh_generic_server_cb_param_t *param)
{
    switch (event) {
        case ESP_BLE_MESH_GENERIC_SERVER_STATE_CHANGE_EVT:
            ESP_LOGI(TAG, "Server: State changed");
            if (param->ctx.recv_op == ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_SET ||
                param->ctx.recv_op == ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_SET_UNACK) {
                onoff_state = param->value.state_change.onoff_set.onoff;
                ESP_LOGI(TAG, "Server: OnOff state changed to %d (from addr 0x%04x)", 
                         onoff_state, param->ctx.addr);
            }
            break;
        case ESP_BLE_MESH_GENERIC_SERVER_RECV_GET_MSG_EVT:
            ESP_LOGI(TAG, "Server: Received GET message from 0x%04x", param->ctx.addr);
            break;
        case ESP_BLE_MESH_GENERIC_SERVER_RECV_SET_MSG_EVT:
            ESP_LOGI(TAG, "Server: Received SET message from 0x%04x", param->ctx.addr);
            if (param->ctx.recv_op == ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_SET ||
                param->ctx.recv_op == ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_SET_UNACK) {
                onoff_state = param->value.set.onoff.onoff;
                ESP_LOGI(TAG, "Server: Set OnOff=%d (TID: %d)", 
                         onoff_state, param->value.set.onoff.tid);
            }
            break;
        default:
            break;
    }
}

void ble_mesh_node_send_onoff(uint8_t onoff, uint16_t addr)
{
    if (!is_provisioned) {
        ESP_LOGW(TAG, "Node not provisioned yet, cannot send messages");
        return;
    }

    esp_ble_mesh_generic_client_set_state_t set_state = {0};
    esp_ble_mesh_client_common_param_t common = {0};
    esp_err_t err;

    // Find the Generic OnOff Client model (index 2 in models array)
    common.opcode = ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_SET_UNACK;
    common.model = &models[2];  // Generic OnOff Client
    common.ctx.net_idx = 0x0000;
    common.ctx.app_idx = APP_KEY_IDX;
    common.ctx.addr = addr;
    common.ctx.send_ttl = 3;
    common.msg_timeout = 0;

    set_state.onoff_set.op_en = false;
    set_state.onoff_set.onoff = onoff;
    set_state.onoff_set.tid = 0;

    err = esp_ble_mesh_generic_client_set_state(&common, &set_state);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send OnOff (err %d)", err);
    } else {
        ESP_LOGI(TAG, "Client: Sent OnOff=%d to 0x%04x", onoff, addr);
    }
}

uint8_t ble_mesh_node_get_onoff(void)
{
    return onoff_state;
}

void ble_mesh_node_set_onoff(uint8_t onoff)
{
    onoff_state = onoff;
    ESP_LOGI(TAG, "Server: OnOff state manually set to %d", onoff_state);
}

bool ble_mesh_node_is_provisioned(void)
{
    return is_provisioned;
}

uint16_t ble_mesh_node_get_addr(void)
{
    return node_addr;
}

esp_err_t ble_mesh_node_init(void)
{
    ESP_LOGI(TAG, "Initializing combined client/server node...");
    esp_err_t err;

    ble_mesh_get_dev_uuid(dev_uuid);
    ESP_LOGI(TAG, "Device UUID: %02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
             dev_uuid[0], dev_uuid[1], dev_uuid[2], dev_uuid[3],
             dev_uuid[4], dev_uuid[5], dev_uuid[6], dev_uuid[7],
             dev_uuid[8], dev_uuid[9], dev_uuid[10], dev_uuid[11],
             dev_uuid[12], dev_uuid[13], dev_uuid[14], dev_uuid[15]);

    // Register all callbacks
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

    err = esp_ble_mesh_register_generic_server_callback(mesh_generic_server_cb);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register generic server callback (err %d)", err);
        return err;
    }

    // Initialize BLE Mesh with composition containing both client and server
    err = esp_ble_mesh_init(&prov, &composition);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "BLE Mesh init failed (err %d)", err);
        return err;
    }

    // Enable node provisioning
    err = esp_ble_mesh_node_prov_enable(
        (esp_ble_mesh_prov_bearer_t)(ESP_BLE_MESH_PROV_ADV | ESP_BLE_MESH_PROV_GATT));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable node provisioning (err %d)", err);
        return err;
    }

    ESP_LOGI(TAG, "BLE Mesh Node initialized successfully!");
    ESP_LOGI(TAG, "This node can:");
    ESP_LOGI(TAG, "  - SEND OnOff commands (client)");
    ESP_LOGI(TAG, "  - RECEIVE OnOff commands (server)");
    ESP_LOGI(TAG, "  - RELAY messages between nodes (mesh routing)");
    ESP_LOGI(TAG, "Ready for provisioning...");

    return ESP_OK;
}
