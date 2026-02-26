/**
 * @file provisioner_node.c
 * @brief Combined provisioner + node (client + server) initialization
 *
 * Merges provisioner models (CFG_SRV, CFG_CLI) with all generic client
 * and server models into a single composition.  esp_ble_mesh_init() is
 * called once in provisioner mode; the device is self-provisioned at
 * PROV_OWN_ADDR and can discover / provision other devices while also
 * participating as a full application-level node.
 */

#include "provisioner_node.h"
#include "provisioner.h"
#include "common/common.h"
#include "common/bluetooth.h"
#include "client/client.h"
#include "server/server.h"
#include "server/server_wrapper.h"

#define TAG                 "PROV_NODE"

#define CID_ESP             0x02E5

#define PROV_OWN_ADDR       0x0001

#define MSG_SEND_TTL        3
#define MSG_TIMEOUT         4000
#define MSG_ROLE            ROLE_PROVISIONER

#define COMP_DATA_PAGE_0    0x00

#define APP_KEY_IDX         0x0000
#define APP_KEY_OCTET       0x12

/* --------------- provisioner bookkeeping (same as provisioner.c) ---------- */

static uint8_t dev_uuid[16] = {0};

static esp_ble_mesh_node_info_t nodes[CONFIG_BLE_MESH_MAX_PROV_NODES] = {};

static esp_ble_mesh_prov_key_t prov_key = {};

/* --------------- merged composition --------------------------------------- */

/* Config Server + Config Client */
static esp_ble_mesh_cfg_srv_t config_server = {
    .net_transmit = ESP_BLE_MESH_TRANSMIT(2, 20),
    .relay = ESP_BLE_MESH_RELAY_DISABLED,
    .relay_retransmit = ESP_BLE_MESH_TRANSMIT(2, 20),
    .beacon = ESP_BLE_MESH_BEACON_ENABLED,
    .gatt_proxy = ESP_BLE_MESH_GATT_PROXY_DISABLED,
    .friend_state = ESP_BLE_MESH_FRIEND_NOT_SUPPORTED,
    .default_ttl = 7,
};

static esp_ble_mesh_client_t config_client;

/* Generic clients */
static esp_ble_mesh_client_t onoff_client;
static esp_ble_mesh_client_t level_client;
static esp_ble_mesh_client_t def_trans_time_client;
static esp_ble_mesh_client_t power_onoff_client;
static esp_ble_mesh_client_t power_level_client;
static esp_ble_mesh_client_t battery_client;
static esp_ble_mesh_client_t location_client;
static esp_ble_mesh_client_t property_client;

/* Generic servers (wrapped in mesh_server_t for callback dispatch) */
static mesh_server_t onoff_server_wrapper;
static esp_ble_mesh_gen_level_srv_t level_server;
static esp_ble_mesh_gen_def_trans_time_srv_t def_trans_time_server;

/* Publication contexts (one per publishable model, can be shared/NULL) */
static esp_ble_mesh_model_pub_t onoff_cli_pub;
static esp_ble_mesh_model_pub_t level_cli_pub;
static esp_ble_mesh_model_pub_t dtt_cli_pub;
static esp_ble_mesh_model_pub_t pou_cli_pub;
static esp_ble_mesh_model_pub_t pl_cli_pub;
static esp_ble_mesh_model_pub_t bat_cli_pub;
static esp_ble_mesh_model_pub_t loc_cli_pub;
static esp_ble_mesh_model_pub_t prop_cli_pub;

static esp_ble_mesh_model_pub_t onoff_srv_pub;
static esp_ble_mesh_model_pub_t level_srv_pub;
static esp_ble_mesh_model_pub_t dtt_srv_pub;

/*
 * Combined model array: provisioner models + all generic client + server models.
 * CFG_SRV and CFG_CLI are mandatory for a provisioner.  The remaining models
 * give the device full client+server capability for Generic OnOff, Level,
 * Default Transition Time, and more.
 */
static esp_ble_mesh_model_t root_models[] = {
    /* Provisioner foundation models */
    ESP_BLE_MESH_MODEL_CFG_SRV(&config_server),
    ESP_BLE_MESH_MODEL_CFG_CLI(&config_client),

    /* Generic client models */
    ESP_BLE_MESH_MODEL_GEN_ONOFF_CLI(&onoff_cli_pub, &onoff_client),
    ESP_BLE_MESH_MODEL_GEN_LEVEL_CLI(&level_cli_pub, &level_client),
    ESP_BLE_MESH_MODEL_GEN_DEF_TRANS_TIME_CLI(&dtt_cli_pub, &def_trans_time_client),
    ESP_BLE_MESH_MODEL_GEN_POWER_ONOFF_CLI(&pou_cli_pub, &power_onoff_client),
    ESP_BLE_MESH_MODEL_GEN_POWER_LEVEL_CLI(&pl_cli_pub, &power_level_client),
    ESP_BLE_MESH_MODEL_GEN_BATTERY_CLI(&bat_cli_pub, &battery_client),
    ESP_BLE_MESH_MODEL_GEN_LOCATION_CLI(&loc_cli_pub, &location_client),
    ESP_BLE_MESH_MODEL_GEN_PROPERTY_CLI(&prop_cli_pub, &property_client),

    /* Generic server models */
    ESP_BLE_MESH_MODEL_GEN_ONOFF_SRV(&onoff_srv_pub, &onoff_server_wrapper.srv.onoff),
    ESP_BLE_MESH_MODEL_GEN_LEVEL_SRV(&level_srv_pub, &level_server),
    ESP_BLE_MESH_MODEL_GEN_DEF_TRANS_TIME_SRV(&dtt_srv_pub, &def_trans_time_server),
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
    .uuid                = dev_uuid,
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

/* --------------- provisioner helpers (from provisioner.c) ----------------- */

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

/* --------------- provisioning complete: config remote node ---------------- */

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

/* --------------- provisioning link helpers -------------------------------- */

static void prov_link_open(esp_ble_mesh_prov_bearer_t bearer)
{
    ESP_LOGI(TAG, "%s link open", bearer == ESP_BLE_MESH_PROV_ADV ? "PB-ADV" : "PB-GATT");
}

static void prov_link_close(esp_ble_mesh_prov_bearer_t bearer, uint8_t reason)
{
    ESP_LOGI(TAG, "%s link close, reason 0x%02x",
             bearer == ESP_BLE_MESH_PROV_ADV ? "PB-ADV" : "PB-GATT", reason);
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

    ESP_LOGI(TAG, "address: %s, address type: %d, adv type: %d",
             bt_hex(addr, BD_ADDR_LEN), addr_type, adv_type_param);
    ESP_LOGI(TAG, "device uuid: %s", bt_hex(dev_uuid_param, 16));
    ESP_LOGI(TAG, "oob info: %d, bearer: %s", oob_info,
             (bearer & ESP_BLE_MESH_PROV_ADV) ? "PB-ADV" : "PB-GATT");

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

/* --------------- provisioning callback ------------------------------------ */

static void ble_mesh_provisioning_cb(esp_ble_mesh_prov_cb_event_t event,
                                     esp_ble_mesh_prov_cb_param_t *param)
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
            recv_unprov_adv_pkt(
                param->provisioner_recv_unprov_adv_pkt.dev_uuid,
                param->provisioner_recv_unprov_adv_pkt.addr,
                param->provisioner_recv_unprov_adv_pkt.addr_type,
                param->provisioner_recv_unprov_adv_pkt.oob_info,
                param->provisioner_recv_unprov_adv_pkt.adv_type,
                param->provisioner_recv_unprov_adv_pkt.bearer);
            break;
        case ESP_BLE_MESH_PROVISIONER_PROV_LINK_OPEN_EVT:
            prov_link_open(param->provisioner_prov_link_open.bearer);
            break;
        case ESP_BLE_MESH_PROVISIONER_PROV_LINK_CLOSE_EVT:
            prov_link_close(param->provisioner_prov_link_close.bearer,
                            param->provisioner_prov_link_close.reason);
            break;
        case ESP_BLE_MESH_PROVISIONER_PROV_COMPLETE_EVT:
            prov_complete(
                param->provisioner_prov_complete.node_idx,
                param->provisioner_prov_complete.device_uuid,
                param->provisioner_prov_complete.unicast_addr,
                param->provisioner_prov_complete.element_num,
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
                name = esp_ble_mesh_provisioner_get_node_name(
                    param->provisioner_set_node_name_comp.node_index);
                if (!name)
                {
                    ESP_LOGE(TAG, "Get node name failed");
                    return;
                }
                ESP_LOGI(TAG, "Node %d name is: %s",
                         param->provisioner_set_node_name_comp.node_index, name);
            }
            break;
        }
        case ESP_BLE_MESH_PROVISIONER_ADD_LOCAL_APP_KEY_COMP_EVT:
        {
            ESP_LOGI(TAG, "ESP_BLE_MESH_PROVISIONER_ADD_LOCAL_APP_KEY_COMP_EVT, err_code %d",
                param->provisioner_add_app_key_comp.err_code);

            if (param->provisioner_add_app_key_comp.err_code == ESP_OK)
            {
                prov_key.app_idx = param->provisioner_add_app_key_comp.app_idx;
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

/* --------------- config client callback (provisioner-style) --------------- */

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
             __func__, param->error_code, event, addr, opcode);

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
                    ESP_LOGI(TAG, "composition data %s",
                             bt_hex(param->status_cb.comp_data_status.composition_data->data,
                                    param->status_cb.comp_data_status.composition_data->len));
                    esp_ble_mesh_cfg_client_set_state_t set_state = {};
                    ble_mesh_set_msg_common(&common, node, config_client.model,
                                           ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD);
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
                    ble_mesh_set_msg_common(&common, node, config_client.model,
                                           ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND);
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
                case ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND:
                {
                    ESP_LOGI(TAG, "ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND event");
                    break;
                }
                default:
                    break;
            }
            break;
        case ESP_BLE_MESH_CFG_CLIENT_PUBLISH_EVT:
            switch (opcode)
            {
                case ESP_BLE_MESH_MODEL_OP_COMPOSITION_DATA_STATUS:
                    ESP_LOG_BUFFER_HEX("composition data %s",
                        param->status_cb.comp_data_status.composition_data->data,
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
                    ble_mesh_set_msg_common(&common, node, config_client.model,
                                           ESP_BLE_MESH_MODEL_OP_COMPOSITION_DATA_GET);
                    get_state.comp_data_get.page = COMP_DATA_PAGE_0;
                    error = esp_ble_mesh_config_client_get_state(&common, &get_state);
                    if (error)
                    {
                        ESP_LOGE(TAG, "%s: Config Composition Data Get failed", __func__);
                        return;
                    }
                    break;
                }
                case ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD:
                {
                    esp_ble_mesh_cfg_client_set_state_t set_state = {};
                    ble_mesh_set_msg_common(&common, node, config_client.model,
                                           ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD);
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
                case ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND:
                {
                    esp_ble_mesh_cfg_client_set_state_t set_state = {};
                    ble_mesh_set_msg_common(&common, node, config_client.model,
                                           ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND);
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

/* --------------- local AppKey binding helper ------------------------------ */

/**
 * Bind AppKey to every application model (skip CFG_SRV and CFG_CLI which
 * don't use application keys).
 */
static esp_err_t bind_app_key_to_local_models(void)
{
    esp_err_t error;

    for (int e = 0; e < composition.element_count; e++)
    {
        esp_ble_mesh_elem_t *elem = &composition.elements[e];
        for (int m = 0; m < elem->sig_model_count; m++)
        {
            uint16_t model_id = elem->sig_models[m].model_id;

            /* Foundation models (0x0000 = CFG SRV, 0x0001 = CFG CLI) don't use AppKey */
            if (model_id == ESP_BLE_MESH_MODEL_ID_CONFIG_SRV ||
                model_id == ESP_BLE_MESH_MODEL_ID_CONFIG_CLI)
            {
                continue;
            }

            error = esp_ble_mesh_provisioner_bind_app_key_to_local_model(
                PROV_OWN_ADDR + e, /* element address */
                prov_key.app_idx,
                model_id,
                ESP_BLE_MESH_CID_NVAL);
            if (error != ESP_OK)
            {
                ESP_LOGE(TAG, "Failed to bind AppKey to model 0x%04x on element %d (err %d)",
                         model_id, e, error);
                return error;
            }
            ESP_LOGI(TAG, "Bound AppKey to local model 0x%04x on element %d", model_id, e);
        }
    }

    return ESP_OK;
}

/* --------------- wire server callbacks ------------------------------------ */

/**
 * Walk the composition and set the user-provided callback on every server
 * model that uses a mesh_server_t wrapper (same pattern as node.c:88-108).
 */
static void wire_server_callbacks(mesh_server_evt_cb_t cb)
{
    for (int e = 0; e < composition.element_count; e++)
    {
        esp_ble_mesh_elem_t *elem = &composition.elements[e];
        if (!elem || !elem->sig_models) continue;

        for (int m = 0; m < elem->sig_model_count; m++)
        {
            esp_ble_mesh_model_t *model = &elem->sig_models[m];

            /* Skip client models (they don't have mesh_server_t user_data) */
            if (model->model_id == ESP_BLE_MESH_MODEL_ID_GEN_ONOFF_CLI ||
                model->model_id == ESP_BLE_MESH_MODEL_ID_GEN_LEVEL_CLI ||
                model->model_id == ESP_BLE_MESH_MODEL_ID_GEN_DEF_TRANS_TIME_CLI ||
                model->model_id == ESP_BLE_MESH_MODEL_ID_GEN_POWER_ONOFF_CLI ||
                model->model_id == ESP_BLE_MESH_MODEL_ID_GEN_POWER_LEVEL_CLI ||
                model->model_id == ESP_BLE_MESH_MODEL_ID_GEN_BATTERY_CLI ||
                model->model_id == ESP_BLE_MESH_MODEL_ID_GEN_LOCATION_CLI ||
                model->model_id == ESP_BLE_MESH_MODEL_ID_GEN_PROP_CLI ||
                model->model_id == ESP_BLE_MESH_MODEL_ID_CONFIG_SRV ||
                model->model_id == ESP_BLE_MESH_MODEL_ID_CONFIG_CLI)
            {
                continue;
            }

            if (model->user_data)
            {
                ((mesh_server_t *)model->user_data)->cb = cb;
            }
        }
    }
}

/* --------------- public API ----------------------------------------------- */

esp_err_t ble_mesh_provisioner_node_init(mesh_server_evt_cb_t cb)
{
    uint8_t match[2] = {0xdd, 0xdd};
    esp_err_t error = ESP_OK;

    ESP_LOGI(TAG, "Initializing combined provisioner + node...");

    /* Wire server callbacks before mesh init */
    wire_server_callbacks(cb);

    /* Setup provisioner keys */
    prov_key.net_idx = ESP_BLE_MESH_KEY_PRIMARY;
    prov_key.app_idx = APP_KEY_IDX;
    memset(prov_key.app_key, APP_KEY_OCTET, sizeof(prov_key.app_key));

    /* Register ALL callbacks */
    error = esp_ble_mesh_register_prov_callback(ble_mesh_provisioning_cb);
    if (error != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to register prov callback (err %d)", error);
        return error;
    }

    error = esp_ble_mesh_register_config_client_callback(ble_mesh_config_client_cb);
    if (error != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to register config client callback (err %d)", error);
        return error;
    }

    error = esp_ble_mesh_register_config_server_callback(mesh_config_server_cb);
    if (error != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to register config server callback (err %d)", error);
        return error;
    }

    error = esp_ble_mesh_register_generic_client_callback(mesh_generic_client_cb);
    if (error != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to register generic client callback (err %d)", error);
        return error;
    }

    error = esp_ble_mesh_register_generic_server_callback(mesh_generic_server_cb);
    if (error != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to register generic server callback (err %d)", error);
        return error;
    }

    /* Get device UUID */
    ble_mesh_get_dev_uuid(dev_uuid);
    ESP_LOGI(TAG, "Device UUID: %02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
             dev_uuid[0], dev_uuid[1], dev_uuid[2], dev_uuid[3],
             dev_uuid[4], dev_uuid[5], dev_uuid[6], dev_uuid[7],
             dev_uuid[8], dev_uuid[9], dev_uuid[10], dev_uuid[11],
             dev_uuid[12], dev_uuid[13], dev_uuid[14], dev_uuid[15]);

    /* Initialize BLE Mesh stack (ONCE, in provisioner mode) */
    error = esp_ble_mesh_init(&provision, &composition);
    if (error != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize mesh stack (err %d)", error);
        return error;
    }

    /* Set UUID match filter for auto-provisioning */
    error = esp_ble_mesh_provisioner_set_dev_uuid_match(match, sizeof(match), 0x0, false);
    if (error != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to set matching device uuid (err %d)", error);
        return error;
    }

    /* Enable provisioning on ADV + GATT bearers */
    error = esp_ble_mesh_provisioner_prov_enable(
        (esp_ble_mesh_prov_bearer_t)(ESP_BLE_MESH_PROV_ADV | ESP_BLE_MESH_PROV_GATT));
    if (error != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to enable mesh provisioner (err %d)", error);
        return error;
    }

    /* Add local AppKey */
    error = esp_ble_mesh_provisioner_add_local_app_key(
        prov_key.app_key, prov_key.net_idx, prov_key.app_idx);
    if (error != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to add local AppKey (err %d)", error);
        return error;
    }

    /* Bind AppKey to all local application models */
    error = bind_app_key_to_local_models();
    if (error != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to bind AppKey to local models (err %d)", error);
        return error;
    }

    /*
     * Tell the client module about our composition and that we are
     * already provisioned, so all client send functions work immediately.
     */
    ble_mesh_client_set_composition(&composition);
    ble_mesh_client_set_provisioned(true);

    ESP_LOGI(TAG, "BLE Mesh Provisioner+Node initialized successfully");

    return ESP_OK;
}
