#include "provisioner.h"
#include "common.h"

#define TAG                 "PROVISIONER"

#define CID_ESP             0x02E5

#define PROV_OWN_ADDR       0x0001

#define MSG_SEND_TTL        3
#define MSG_TIMEOUT         0
#define MSG_ROLE            ROLE_PROVISIONER

#define COMP_DATA_PAGE_0    0x00

#define APP_KEY_IDX         0x0000
#define APP_KEY_OCTET       0x12

static uint8_t dev_uuid[16];

static esp_ble_mesh_node_info_t nodes[CONFIG_BLE_MESH_MAX_PROV_NODES] = {};

static esp_ble_mesh_prov_key_t prov_key = {};

static esp_ble_mesh_client_t config_client;

static esp_ble_mesh_cfg_srv_t config_server = {
    .net_transmit = ESP_BLE_MESH_TRANSMIT(2, 20),
    .relay = ESP_BLE_MESH_RELAY_DISABLED,
    .relay_retransmit = ESP_BLE_MESH_TRANSMIT(2, 20),
    .beacon = ESP_BLE_MESH_BEACON_ENABLED,
    .gatt_proxy = ESP_BLE_MESH_GATT_PROXY_ENABLED,
    .friend_state = ESP_BLE_MESH_FRIEND_NOT_SUPPORTED,
    .default_ttl = 7
};

static esp_ble_mesh_model_t root_models[] = {
    ESP_BLE_MESH_MODEL_CFG_SRV(&config_server),
    ESP_BLE_MESH_MODEL_CFG_CLI(&config_client),
};

static esp_ble_mesh_elem_t elements[] = {
    ESP_BLE_MESH_ELEMENT(0, root_models, ESP_BLE_MESH_MODEL_NONE),
};

static esp_ble_mesh_comp_t composition = {
    .cid = CID_ESP,
    .element_count = ARRAY_SIZE(elements),
    .elements = elements,
};

static esp_ble_mesh_prov_t provision = {
    .prov_uuid           = dev_uuid,
    .prov_unicast_addr   = PROV_OWN_ADDR,
    .prov_start_address  = 0x0005,
    .prov_attention      = 0x00,
    .prov_algorithm      = 0x00,
    .prov_pub_key_oob    = 0x00,
    .prov_static_oob_val = NULL,
    .prov_static_oob_len = 0x00,
    .flags               = 0x00,
    .iv_index            = 0x00,
};

static esp_err_t ble_mesh_store_node_info(const uint8_t uuid[16], uint16_t unicast, uint8_t elem_num)
{
    if (!uuid || !ESP_BLE_MESH_ADDR_IS_UNICAST(unicast))
    {
        return ESP_ERR_INVALID_ARG;
    }

    for (int i = 0; i < ARRAY_SIZE(nodes); i++)
    {
        if (!memcmp(nodes[i].uuid, uuid, 16))
        {
            ESP_LOGW(TAG, "%s: reprovisioned device 0x%04x", __func__, unicast);
            nodes[i].unicast = unicast;
            nodes[i].elem_num = elem_num;
            return ESP_OK;
        }
    }

    for (int i = 0; i < ARRAY_SIZE(nodes); i++)
    {
        if (nodes[i].unicast == ESP_BLE_MESH_ADDR_UNASSIGNED)
        {
            memcpy(nodes[i].uuid, uuid, 16);
            nodes[i].unicast = unicast;
            nodes[i].elem_num = elem_num;
            return ESP_OK;
        }
    }

    return ESP_FAIL;
}

static esp_ble_mesh_node_info_t *ble_mesh_get_node_info(uint16_t unicast)
{
    if (!ESP_BLE_MESH_ADDR_IS_UNICAST(unicast))
    {
        return NULL;
    }

    for (int i = 0; i < ARRAY_SIZE(nodes); i++)
    {
        if (nodes[i].unicast <= unicast && nodes[i].unicast + nodes[i].elem_num > unicast)
        {
            return &nodes[i];
        }
    }

    return NULL;
}

static esp_err_t ble_mesh_set_msg_common(
    esp_ble_mesh_client_common_param_t *common,
    esp_ble_mesh_node_info_t *node,
    esp_ble_mesh_model_t *model,
    uint32_t opcode)
{
    if (!common || !node || !model)
    {
        return ESP_ERR_INVALID_ARG;
    }

    common->opcode = opcode;
    common->model = model;
    common->ctx.net_idx = prov_key.net_idx;
    common->ctx.app_idx = prov_key.app_idx;
    common->ctx.addr = node->unicast;
    common->ctx.send_ttl = MSG_SEND_TTL;
    common->msg_timeout = MSG_TIMEOUT;

    return ESP_OK;
}

static esp_err_t prov_complete(
    int node_idx,
    const esp_ble_mesh_octet16_t uuid,
    uint16_t unicast,
    uint8_t elem_num,
    uint16_t net_idx)
{
    esp_ble_mesh_client_common_param_t common = {};
    esp_ble_mesh_cfg_client_get_state_t get_state = {};
    esp_ble_mesh_node_info_t *node = NULL;
    char name[11] = {0};
    esp_err_t error;

    ESP_LOGI(TAG, "node index: 0x%x, unicast address: 0x%02x, element num: %d, "
                  "netkey index: 0x%02x", node_idx, unicast, elem_num, net_idx);
    ESP_LOGI(TAG, "device uuid: %s", bt_hex(uuid, 16));

    sprintf(name, "%s%d", "NODE-", node_idx);
    error = esp_ble_mesh_provisioner_set_node_name(node_idx, name);
    if (error)
    {
        ESP_LOGE(TAG, "%s: Set node name failed", __func__);
        return ESP_FAIL;
    }

    error = ble_mesh_store_node_info(uuid, unicast, elem_num);
    if (error)
    {
        ESP_LOGE(TAG, "%s: Store node info failed", __func__);
        return ESP_FAIL;
    }

    node = ble_mesh_get_node_info(unicast);
    if (!node)
    {
        ESP_LOGE(TAG, "%s: Get node info failed", __func__);
        return ESP_FAIL;
    }

    ble_mesh_set_msg_common(&common, node, config_client.model, ESP_BLE_MESH_MODEL_OP_COMPOSITION_DATA_GET);
    get_state.comp_data_get.page = COMP_DATA_PAGE_0;
    error = esp_ble_mesh_config_client_get_state(&common, &get_state);
    if (error)
    {
        ESP_LOGE(TAG, "%s: Send config comp data get failed", __func__);
        return ESP_FAIL;
    }

    return ESP_OK;
}

static void prov_link_open(esp_ble_mesh_prov_bearer_t bearer)
{
    ESP_LOGI(TAG, "%s link open", bearer == ESP_BLE_MESH_PROV_ADV ? "PB-ADV" : "PB-GATT");
}

static void prov_link_close(esp_ble_mesh_prov_bearer_t bearer, uint8_t reason)
{
    ESP_LOGI(TAG, "%s link close, reason 0x%02x", bearer == ESP_BLE_MESH_PROV_ADV ? "PB-ADV" : "PB-GATT", reason);
}

static void recv_unprov_adv_pkt(
    uint8_t dev_uuid_param[16],
    uint8_t addr[BD_ADDR_LEN],
    esp_ble_mesh_addr_type_t addr_type,
    uint16_t oob_info, uint8_t adv_type_param,
    esp_ble_mesh_prov_bearer_t bearer)
{
    esp_ble_mesh_unprov_dev_add_t add_dev = {};
    esp_err_t error;

    ESP_LOGI(TAG, "address: %s, address type: %d, adv type: %d", bt_hex(addr, BD_ADDR_LEN), addr_type, adv_type_param);
    ESP_LOGI(TAG, "device uuid: %s", bt_hex(dev_uuid_param, 16));
    ESP_LOGI(TAG, "oob info: %d, bearer: %s", oob_info, (bearer & ESP_BLE_MESH_PROV_ADV) ? "PB-ADV" : "PB-GATT");

    memcpy(add_dev.addr, addr, BD_ADDR_LEN);
    add_dev.addr_type = addr_type;
    memcpy(add_dev.uuid, dev_uuid_param, 16);
    add_dev.oob_info = oob_info;
    add_dev.bearer = bearer;

    error = esp_ble_mesh_provisioner_add_unprov_dev(&add_dev,
        (ADD_DEV_RM_AFTER_PROV_FLAG | ADD_DEV_START_PROV_NOW_FLAG | ADD_DEV_FLUSHABLE_DEV_FLAG));
    if (error)
    {
        ESP_LOGE(TAG, "%s: Add unprovisioned device into queue failed", __func__);
    }
}

static void ble_mesh_provisioning_cb(esp_ble_mesh_prov_cb_event_t event, esp_ble_mesh_prov_cb_param_t *param)
{
    switch (event)
    {
        case ESP_BLE_MESH_PROVISIONER_PROV_ENABLE_COMP_EVT:
            ESP_LOGI(TAG, "ESP_BLE_MESH_PROVISIONER_PROV_ENABLE_COMP_EVT, err_code %d",
                param->provisioner_prov_enable_comp.err_code);
            break;
        case ESP_BLE_MESH_PROVISIONER_PROV_DISABLE_COMP_EVT:
            ESP_LOGI(TAG, "ESP_BLE_MESH_PROVISIONER_PROV_DISABLE_COMP_EVT, err_code %d",
                param->provisioner_prov_disable_comp.err_code);
            break;
        case ESP_BLE_MESH_PROVISIONER_RECV_UNPROV_ADV_PKT_EVT:
            ESP_LOGI(TAG, "ESP_BLE_MESH_PROVISIONER_RECV_UNPROV_ADV_PKT_EVT");
            recv_unprov_adv_pkt(param->provisioner_recv_unprov_adv_pkt.dev_uuid, param->provisioner_recv_unprov_adv_pkt.addr,
                                param->provisioner_recv_unprov_adv_pkt.addr_type, param->provisioner_recv_unprov_adv_pkt.oob_info,
                                param->provisioner_recv_unprov_adv_pkt.adv_type, param->provisioner_recv_unprov_adv_pkt.bearer);
            break;
        case ESP_BLE_MESH_PROVISIONER_PROV_LINK_OPEN_EVT:
            prov_link_open(param->provisioner_prov_link_open.bearer);
            break;
        case ESP_BLE_MESH_PROVISIONER_PROV_LINK_CLOSE_EVT:
            prov_link_close(param->provisioner_prov_link_close.bearer, param->provisioner_prov_link_close.reason);
            break;
        case ESP_BLE_MESH_PROVISIONER_PROV_COMPLETE_EVT:
            prov_complete(param->provisioner_prov_complete.node_idx, param->provisioner_prov_complete.device_uuid,
                          param->provisioner_prov_complete.unicast_addr, param->provisioner_prov_complete.element_num,
                          param->provisioner_prov_complete.netkey_idx);
            break;
        case ESP_BLE_MESH_PROVISIONER_ADD_UNPROV_DEV_COMP_EVT:
            ESP_LOGI(TAG, "ESP_BLE_MESH_PROVISIONER_ADD_UNPROV_DEV_COMP_EVT, err_code %d",
                param->provisioner_add_unprov_dev_comp.err_code);
            break;
        case ESP_BLE_MESH_PROVISIONER_SET_DEV_UUID_MATCH_COMP_EVT:
            ESP_LOGI(TAG, "ESP_BLE_MESH_PROVISIONER_SET_DEV_UUID_MATCH_COMP_EVT, err_code %d",
                param->provisioner_set_dev_uuid_match_comp.err_code);
            break;
        case ESP_BLE_MESH_PROVISIONER_SET_NODE_NAME_COMP_EVT:
        {
            ESP_LOGI(TAG, "ESP_BLE_MESH_PROVISIONER_SET_NODE_NAME_COMP_EVT, err_code %d",
                param->provisioner_set_node_name_comp.err_code);

            if (param->provisioner_set_node_name_comp.err_code == ESP_OK)
            {
                const char *name = NULL;
                name = esp_ble_mesh_provisioner_get_node_name(param->provisioner_set_node_name_comp.node_index);
                if (!name)
                {
                    ESP_LOGE(TAG, "Get node name failed");
                    return;
                }
                ESP_LOGI(TAG, "Node %d name is: %s", param->provisioner_set_node_name_comp.node_index, name);
            }
            break;
        }
        case ESP_BLE_MESH_PROVISIONER_ADD_LOCAL_APP_KEY_COMP_EVT:
        {
            ESP_LOGI(TAG, "ESP_BLE_MESH_PROVISIONER_ADD_LOCAL_APP_KEY_COMP_EVT, err_code %d",
                param->provisioner_add_app_key_comp.err_code);

            if (param->provisioner_add_app_key_comp.err_code == ESP_OK)
            {
                esp_err_t err = 0;
                prov_key.app_idx = param->provisioner_add_app_key_comp.app_idx;
                err = esp_ble_mesh_provisioner_bind_app_key_to_local_model(PROV_OWN_ADDR, prov_key.app_idx,
                        ESP_BLE_MESH_MODEL_ID_GEN_ONOFF_CLI, ESP_BLE_MESH_CID_NVAL);
                if (err != ESP_OK)
                {
                    ESP_LOGE(TAG, "Provisioner bind local model appkey failed");
                    return;
                }
            }
            break;
        }
        case ESP_BLE_MESH_PROVISIONER_BIND_APP_KEY_TO_MODEL_COMP_EVT:
            ESP_LOGI(TAG, "ESP_BLE_MESH_PROVISIONER_BIND_APP_KEY_TO_MODEL_COMP_EVT, err_code %d",
                param->provisioner_bind_app_key_to_model_comp.err_code);
            break;
        default:
            break;
    }
}

static void ble_mesh_config_client_cb(
    esp_ble_mesh_cfg_client_cb_event_t event,
    esp_ble_mesh_cfg_client_cb_param_t *param)
{
    esp_ble_mesh_client_common_param_t common = {};
    esp_ble_mesh_node_info_t *node = NULL;
    uint32_t opcode;
    uint16_t addr;
    esp_err_t error;

    opcode = param->params->opcode;
    addr = param->params->ctx.addr;

    ESP_LOGI(TAG, "%s, error_code = 0x%02x, event = 0x%02x, addr: 0x%04x, opcode: 0x%04" PRIx32,
             __func__, param->error_code, event, param->params->ctx.addr, opcode);

    if (param->error_code)
    {
        ESP_LOGE(TAG, "Send config client message failed, opcode 0x%04" PRIx32, opcode);
        return;
    }

    node = ble_mesh_get_node_info(addr);
    if (!node)
    {
        ESP_LOGE(TAG, "%s: Get node info failed", __func__);
        return;
    }

    switch (event)
    {
        case ESP_BLE_MESH_CFG_CLIENT_GET_STATE_EVT:
            switch (opcode)
            {
                case ESP_BLE_MESH_MODEL_OP_COMPOSITION_DATA_GET:
                {
                    ESP_LOGI(TAG, "composition data %s", bt_hex(param->status_cb.comp_data_status.composition_data->data,
                             param->status_cb.comp_data_status.composition_data->len));
                    esp_ble_mesh_cfg_client_set_state_t set_state = {};
                    ble_mesh_set_msg_common(&common, node, config_client.model, ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD);
                    set_state.app_key_add.net_idx = prov_key.net_idx;
                    set_state.app_key_add.app_idx = prov_key.app_idx;
                    memcpy(set_state.app_key_add.app_key, prov_key.app_key, 16);
                    error = esp_ble_mesh_config_client_set_state(&common, &set_state);
                    if (error)
                    {
                        ESP_LOGE(TAG, "%s: Config AppKey Add failed", __func__);
                        return;
                    }
                    break;
                }
                default:
                    break;
            }
            break;
        case ESP_BLE_MESH_CFG_CLIENT_SET_STATE_EVT:
            switch (opcode)
            {
                case ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD:
                {
                    esp_ble_mesh_cfg_client_set_state_t set_state = {};
                    ble_mesh_set_msg_common(&common, node, config_client.model, ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND);
                    set_state.model_app_bind.element_addr = node->unicast;
                    set_state.model_app_bind.model_app_idx = prov_key.app_idx;
                    set_state.model_app_bind.model_id = ESP_BLE_MESH_MODEL_ID_GEN_ONOFF_SRV;
                    set_state.model_app_bind.company_id = ESP_BLE_MESH_CID_NVAL;
                    error = esp_ble_mesh_config_client_set_state(&common, &set_state);
                    if (error)
                    {
                        ESP_LOGE(TAG, "%s: Config Model App Bind failed", __func__);
                        return;
                    }
                    break;
                }
                /*
                case ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND:
                {
                    esp_ble_mesh_generic_client_get_state_t get_state = {};
                    ble_mesh_set_msg_common(&common, node, onoff_client.model, ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_GET);
                    error = esp_ble_mesh_generic_client_get_state(&common, &get_state);
                    if (error)
                    {
                        ESP_LOGE(TAG, "%s: Generic OnOff Get failed", __func__);
                        return;
                    }
                    break;
                }
                */
                default:
                    break;
            }
            break;
        case ESP_BLE_MESH_CFG_CLIENT_PUBLISH_EVT:
            switch (opcode)
            {
                case ESP_BLE_MESH_MODEL_OP_COMPOSITION_DATA_STATUS:
                    ESP_LOG_BUFFER_HEX("composition data %s", param->status_cb.comp_data_status.composition_data->data,
                                       param->status_cb.comp_data_status.composition_data->len);
                    break;
                case ESP_BLE_MESH_MODEL_OP_APP_KEY_STATUS:
                    break;
                default:
                    break;
            }
            break;
        case ESP_BLE_MESH_CFG_CLIENT_TIMEOUT_EVT:
            switch (opcode)
            {
                case ESP_BLE_MESH_MODEL_OP_COMPOSITION_DATA_GET:
                {
                    esp_ble_mesh_cfg_client_get_state_t get_state = {};
                    ble_mesh_set_msg_common(&common, node, config_client.model, ESP_BLE_MESH_MODEL_OP_COMPOSITION_DATA_GET);
                    get_state.comp_data_get.page = COMP_DATA_PAGE_0;
                    error = esp_ble_mesh_config_client_get_state(&common, &get_state);
                    if (error) {
                        ESP_LOGE(TAG, "%s: Config Composition Data Get failed", __func__);
                        return;
                    }
                    break;
                }
                case ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD:
                {
                    esp_ble_mesh_cfg_client_set_state_t set_state = {};
                    ble_mesh_set_msg_common(&common, node, config_client.model, ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD);
                    set_state.app_key_add.net_idx = prov_key.net_idx;
                    set_state.app_key_add.app_idx = prov_key.app_idx;
                    memcpy(set_state.app_key_add.app_key, prov_key.app_key, 16);
                    error = esp_ble_mesh_config_client_set_state(&common, &set_state);
                    if (error) {
                        ESP_LOGE(TAG, "%s: Config AppKey Add failed", __func__);
                        return;
                    }
                    break;
                }
                case ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND: {
                    esp_ble_mesh_cfg_client_set_state_t set_state = {};
                    ble_mesh_set_msg_common(&common, node, config_client.model, ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND);
                    set_state.model_app_bind.element_addr = node->unicast;
                    set_state.model_app_bind.model_app_idx = prov_key.app_idx;
                    set_state.model_app_bind.model_id = ESP_BLE_MESH_MODEL_ID_GEN_ONOFF_SRV;
                    set_state.model_app_bind.company_id = ESP_BLE_MESH_CID_NVAL;
                    error = esp_ble_mesh_config_client_set_state(&common, &set_state);
                    if (error)
                    {
                        ESP_LOGE(TAG, "%s: Config Model App Bind failed", __func__);
                        return;
                    }
                    break;
                }
                default:
                    break;
            }
            break;
        default:
            ESP_LOGE(TAG, "Not a config client status message event");
            break;
    }
}

static esp_err_t ble_mesh_init(void)
{
    uint8_t match[2] = {0xdd, 0xdd};
    esp_err_t error = ESP_OK;

    prov_key.net_idx = ESP_BLE_MESH_KEY_PRIMARY;
    prov_key.app_idx = APP_KEY_IDX;
    memset(prov_key.app_key, APP_KEY_OCTET, sizeof(prov_key.app_key));

    esp_ble_mesh_register_prov_callback(ble_mesh_provisioning_cb);
    esp_ble_mesh_register_config_client_callback(ble_mesh_config_client_cb);

    error = esp_ble_mesh_init(&provision, &composition);
    if (error != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize mesh stack (error %d)", error);
        return error;
    }

    error = esp_ble_mesh_provisioner_set_dev_uuid_match(match, sizeof(match), 0x0, false);
    if (error != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to set matching device uuid (err %d)", error);
        return error;
    }

    error = esp_ble_mesh_provisioner_prov_enable((esp_ble_mesh_prov_bearer_t)(ESP_BLE_MESH_PROV_ADV | ESP_BLE_MESH_PROV_GATT));
    if (error != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to enable mesh provisioner (err %d)", error);
        return error;
    }

    error = esp_ble_mesh_provisioner_add_local_app_key(prov_key.app_key, prov_key.net_idx, prov_key.app_idx);
    if (error != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to add local AppKey (err %d)", error);
        return error;
    }

    ESP_LOGI(TAG, "BLE Mesh Provisioner initialized");

    return error;
}
