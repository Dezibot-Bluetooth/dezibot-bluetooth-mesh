#include "client.h"
#include "../common/bluetooth.h"
#include "../common/common.h"

#define TAG "BLE_MESH_CLIENT"
#define APP_KEY_IDX 0x0000

static uint8_t dev_uuid[16];
static bool is_provisioned = false;
static uint16_t node_addr = 0;

static esp_ble_mesh_comp_t *composition = NULL;

static esp_ble_mesh_prov_t prov = {
    .uuid = dev_uuid,
};

void ble_mesh_client_set_composition(esp_ble_mesh_comp_t *comp)
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
            switch (param->params->opcode)
            {
                case ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_GET:
                {
                    uint8_t onoff = param->status_cb.onoff_status.present_onoff;
                    esp_ble_mesh_gen_onoff_srv_t *srv = param->params->model->user_data;
                    srv->state.onoff = onoff;
                    ESP_LOGI(TAG, "Received OnOff Status: %d", onoff);
                    break;
                }
                case ESP_BLE_MESH_MODEL_OP_GEN_LEVEL_GET:
                {
                    int16_t level = param->status_cb.level_status.present_level;
                    esp_ble_mesh_gen_level_srv_t *srv = param->params->model->user_data;
                    srv->state.level = level;
                    ESP_LOGI(TAG, "Received Level Status: %d", level);
                    break;
                }
                case ESP_BLE_MESH_MODEL_OP_GEN_DEF_TRANS_TIME_GET:
                {
                    uint8_t transition_time = param->status_cb.def_trans_time_status.trans_time;
                    esp_ble_mesh_gen_def_trans_time_srv_t *srv = param->params->model->user_data;
                    srv->state.trans_time = transition_time;
                    ESP_LOGI(TAG, "Received Default Transition Time Status: %d", transition_time);
                    break;
                }
                case ESP_BLE_MESH_MODEL_OP_GEN_ONPOWERUP_GET:
                {
                    uint8_t onpowerup = param->status_cb.onpowerup_status.onpowerup;
                    esp_ble_mesh_gen_power_onoff_srv_t *srv = param->params->model->user_data;
                    srv->state->onpowerup = onpowerup;
                    ESP_LOGI(TAG, "Received OnPowerUp Status: %d", onpowerup);
                    break;
                }
                case ESP_BLE_MESH_MODEL_OP_GEN_POWER_LEVEL_GET:
                {
                    uint16_t power_level = param->status_cb.power_level_status.present_power;
                    esp_ble_mesh_gen_power_level_srv_t *srv = param->params->model->user_data;
                    srv->state->power_actual = power_level;
                    ESP_LOGI(TAG, "Received Power Level Status: %d", power_level);
                    break;
                }
                case ESP_BLE_MESH_MODEL_OP_GEN_POWER_DEFAULT_GET:
                {
                    uint16_t power_default = param->status_cb.power_default_status.power;
                    esp_ble_mesh_gen_power_level_srv_t *srv = param->params->model->user_data;
                    srv->state->power_default = power_default;
                    ESP_LOGI(TAG, "Received Power Default Status: %d", power_default);
                    break;
                }
                case ESP_BLE_MESH_MODEL_OP_GEN_POWER_LAST_GET:
                {
                    uint16_t power_last = param->status_cb.power_last_status.power;
                    esp_ble_mesh_gen_power_level_srv_t *srv = param->params->model->user_data;
                    srv->state->power_last = power_last;
                    ESP_LOGI(TAG, "Received Power Last Status: %d", power_last);
                    break;
                }
                case ESP_BLE_MESH_MODEL_OP_GEN_POWER_RANGE_GET:
                {
                    uint8_t staus_code = param->status_cb.power_range_status.status_code;
                    uint16_t power_range_min = param->status_cb.power_range_status.range_min;
                    uint16_t power_range_max = param->status_cb.power_range_status.range_max;
                    esp_ble_mesh_gen_power_level_srv_t *srv = param->params->model->user_data;
                    srv->state->status_code = staus_code;
                    srv->state->power_range_min = power_range_min;
                    srv->state->power_range_max = power_range_max;
                    ESP_LOGI(TAG, "Received Power Range Status: status %d, min %d, max %d", staus_code, power_range_min, power_range_max);
                    break;
                }
                case ESP_BLE_MESH_MODEL_OP_GEN_BATTERY_GET:
                {
                    uint8_t battery_level = param->status_cb.battery_status.battery_level;
                    uint32_t battery_time_to_discharge = param->status_cb.battery_status.time_to_discharge;
                    uint32_t battery_time_to_charge = param->status_cb.battery_status.time_to_charge;
                    uint8_t battery_flags = param->status_cb.battery_status.flags;
                    esp_ble_mesh_gen_battery_srv_t *srv = param->params->model->user_data;
                    srv->state.battery_level = battery_level;
                    srv->state.time_to_discharge = battery_time_to_discharge;
                    srv->state.time_to_charge = battery_time_to_charge;
                    srv->state.battery_flags = battery_flags;
                    ESP_LOGI(TAG, "Received Battery Status: level %d, discharge %d, charge %d, flags %d", battery_level,
                        battery_time_to_discharge, battery_time_to_charge, battery_flags);
                    break;
                }
                case ESP_BLE_MESH_MODEL_OP_GEN_LOC_GLOBAL_GET:
                {
                    int32_t latitude = param->status_cb.location_global_status.global_latitude;
                    int32_t longitude = param->status_cb.location_global_status.global_longitude;
                    int16_t altitude = param->status_cb.location_global_status.global_altitude;
                    esp_ble_mesh_gen_location_srv_t *srv = param->params->model->user_data;
                    srv->state->global_latitude = latitude;
                    srv->state->global_longitude = longitude;
                    srv->state->global_altitude = altitude;
                    ESP_LOGI(TAG, "Received Global Location Status: latitude %d, longitude %d, altitude %d", latitude, longitude, altitude);
                    break;
                }
                case ESP_BLE_MESH_MODEL_OP_GEN_LOC_LOCAL_GET:
                {
                    int16_t north = param->status_cb.location_local_status.local_north;
                    int16_t east = param->status_cb.location_local_status.local_east;
                    int16_t altitude = param->status_cb.location_local_status.local_altitude;
                    uint8_t floor_number = param->status_cb.location_local_status.floor_number;
                    uint16_t uncertainty = param->status_cb.location_local_status.uncertainty;
                    esp_ble_mesh_gen_location_srv_t *srv = param->params->model->user_data;
                    srv->state->local_north = north;
                    srv->state->local_east = east;
                    srv->state->local_altitude = altitude;
                    srv->state->floor_number = floor_number;
                    srv->state->uncertainty = uncertainty;
                    ESP_LOGI(TAG, "Received Local Location Status: north %d, east %d, altitude %d, floor %d, uncertainty %d",
                        north, east, altitude, floor_number, uncertainty);
                    break;
                }
                default:
                    ESP_LOGI(TAG, "Generic client get state event");
                    break;
            }
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

esp_err_t ble_mesh_client_get_onoff(uint16_t addr, uint8_t elem_index)
{
    esp_ble_mesh_client_common_param_t common = {0};
    esp_ble_mesh_model_t *model = NULL;
    esp_err_t err;

    if (!is_provisioned) {
        ESP_LOGW(TAG, "Device not provisioned yet, cannot send messages");
        return ESP_ERR_INVALID_STATE;
    }

    for (uint8_t i = 0; i < composition->elements[elem_index].sig_model_count; i++) {
        if (composition->elements[elem_index].sig_models[i].model_id == ESP_BLE_MESH_MODEL_ID_GEN_ONOFF_CLI) {
            model = &composition->elements[elem_index].sig_models[i];
            break;
        }
    }

    if (model == NULL) {
        ESP_LOGE(TAG, "Generic OnOff Client model not found in element %d", elem_index);
        return ESP_ERR_INVALID_STATE;
    }

    common.opcode = ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_GET;
    common.model = model;
    common.ctx.net_idx = 0x0000;
    common.ctx.app_idx = 0x0000;
    common.ctx.addr = addr;
    common.ctx.send_ttl = 3;
    common.msg_timeout = 0;

    err = esp_ble_mesh_generic_client_get_state(&common, NULL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send onoff get (err %d)", err);
        return err;
    }

    ESP_LOGI(TAG, "Sent onoff get to addr 0x%04x", addr);
    return ESP_OK;
}

esp_err_t ble_mesh_client_set_onoff(uint8_t val, uint16_t addr, uint8_t elem_index)
{
    esp_ble_mesh_generic_client_set_state_t set_state = {0};
    esp_ble_mesh_client_common_param_t common = {0};
    esp_ble_mesh_model_t *model = NULL;
    esp_err_t err;
    
    if (!is_provisioned) {
        ESP_LOGW(TAG, "Device not provisioned yet, cannot send messages");
        return ESP_ERR_INVALID_STATE;
    }

    for (uint8_t i = 0; i < composition->elements[elem_index].sig_model_count; i++) {
        if (composition->elements[elem_index].sig_models[i].model_id == ESP_BLE_MESH_MODEL_ID_GEN_ONOFF_CLI) {
            model = &composition->elements[elem_index].sig_models[i];
            break;
        }
    }

    if (model == NULL) {
        ESP_LOGE(TAG, "Generic OnOff Client model not found in element %d", elem_index);
        return ESP_ERR_INVALID_STATE;
    }
    
    common.opcode = ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_SET_UNACK;
    common.model = model;
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
        return err;
    }

    ESP_LOGI(TAG, "Sent onoff %d to addr 0x%04x", val, addr);
    return ESP_OK;
}

esp_err_t ble_mesh_client_get_level(uint16_t addr, uint8_t elem_index)
{
    esp_ble_mesh_client_common_param_t common = {0};
    esp_ble_mesh_model_t *model = NULL;
    esp_err_t err;

    if (!is_provisioned) {
        ESP_LOGW(TAG, "Device not provisioned yet, cannot send messages");
        return ESP_ERR_INVALID_STATE;
    }

    for (uint8_t i = 0; i < composition->elements[elem_index].sig_model_count; i++) {
        if (composition->elements[elem_index].sig_models[i].model_id == ESP_BLE_MESH_MODEL_ID_GEN_LEVEL_CLI) {
            model = &composition->elements[elem_index].sig_models[i];
            break;
        }
    }

    if (model == NULL) {
        ESP_LOGE(TAG, "Generic Level Client model not found in element %d", elem_index);
        return ESP_ERR_INVALID_STATE;
    }

    common.opcode = ESP_BLE_MESH_MODEL_OP_GEN_LEVEL_GET;
    common.model = model;
    common.ctx.net_idx = 0x0000;
    common.ctx.app_idx = 0x0000;
    common.ctx.addr = addr;
    common.ctx.send_ttl = 3;
    common.msg_timeout = 0;

    err = esp_ble_mesh_generic_client_get_state(&common, NULL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send level get (err %d)", err);
        return err;
    }

    ESP_LOGI(TAG, "Sent level get to addr 0x%04x", addr);
    return ESP_OK;
}

esp_err_t ble_mesh_client_set_level(int16_t level, uint16_t addr, uint8_t elem_index)
{
    esp_ble_mesh_generic_client_set_state_t set_state = {0};
    esp_ble_mesh_client_common_param_t common = {0};
    esp_ble_mesh_model_t *model = NULL;
    esp_err_t err;
    
    if (!is_provisioned) {
        ESP_LOGW(TAG, "Device not provisioned yet, cannot send messages");
        return ESP_ERR_INVALID_STATE;
    }

    for (uint8_t i = 0; i < composition->elements[elem_index].sig_model_count; i++) {
        if (composition->elements[elem_index].sig_models[i].model_id == ESP_BLE_MESH_MODEL_ID_GEN_LEVEL_CLI) {
            model = &composition->elements[elem_index].sig_models[i];
            break;
        }
    }

    if (model == NULL) {
        ESP_LOGE(TAG, "Generic Level Client model not found in element %d", elem_index);
        return ESP_ERR_INVALID_STATE;
    }
    
    common.opcode = ESP_BLE_MESH_MODEL_OP_GEN_LEVEL_SET_UNACK;
    common.model = model;
    common.ctx.net_idx = 0x0000;
    common.ctx.app_idx = 0x0000;
    common.ctx.addr = addr;
    common.ctx.send_ttl = 3;
    common.msg_timeout = 0;
    
    set_state.level_set.op_en = false;
    set_state.level_set.level = level;
    set_state.level_set.tid = 0;
    
    err = esp_ble_mesh_generic_client_set_state(&common, &set_state);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send level set (err %d)", err);
        return err;
    }

    ESP_LOGI(TAG, "Sent level %d to addr 0x%04x", level, addr);
    return ESP_OK;
}

esp_err_t ble_mesh_client_set_delta(int32_t delta, uint16_t addr, uint8_t elem_index)
{
    esp_ble_mesh_generic_client_set_state_t set_state = {0};
    esp_ble_mesh_client_common_param_t common = {0};
    esp_ble_mesh_model_t *model = NULL;
    esp_err_t err;

    if (!is_provisioned) {
        ESP_LOGW(TAG, "Device not provisioned yet, cannot send messages");
        return ESP_ERR_INVALID_STATE;
    }

    for (uint8_t i = 0; i < composition->elements[elem_index].sig_model_count; i++) {
        if (composition->elements[elem_index].sig_models[i].model_id == ESP_BLE_MESH_MODEL_ID_GEN_LEVEL_CLI) {
            model = &composition->elements[elem_index].sig_models[i];
            break;
        }
    }

    if (model == NULL) {
        ESP_LOGE(TAG, "Generic Level Client model not found in element %d", elem_index);
        return ESP_ERR_INVALID_STATE;
    }

    common.opcode = ESP_BLE_MESH_MODEL_OP_GEN_DELTA_SET_UNACK;
    common.model = model;
    common.ctx.net_idx = 0x0000;
    common.ctx.app_idx = 0x0000;
    common.ctx.addr = addr;
    common.ctx.send_ttl = 3;
    common.msg_timeout = 0;

    set_state.delta_set.op_en = false;
    set_state.delta_set.level = delta;
    set_state.delta_set.tid = 0;

    err = esp_ble_mesh_generic_client_set_state(&common, &set_state);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send delta set (err %d)", err);
        return err;
    }

    ESP_LOGI(TAG, "Sent delta %d to addr 0x%04x", delta, addr);
    return ESP_OK;
}

esp_err_t ble_mesh_client_set_move(int16_t move, uint16_t addr, uint8_t elem_index)
{
    esp_ble_mesh_generic_client_set_state_t set_state = {0};
    esp_ble_mesh_client_common_param_t common = {0};
    esp_ble_mesh_model_t *model = NULL;
    esp_err_t err;

    if (!is_provisioned) {
        ESP_LOGW(TAG, "Device not provisioned yet, cannot send messages");
        return ESP_ERR_INVALID_STATE;
    }

    for (uint8_t i = 0; i < composition->elements[elem_index].sig_model_count; i++) {
        if (composition->elements[elem_index].sig_models[i].model_id == ESP_BLE_MESH_MODEL_ID_GEN_LEVEL_CLI) {
            model = &composition->elements[elem_index].sig_models[i];
            break;
        }
    }

    if (model == NULL) {
        ESP_LOGE(TAG, "Generic Level Client model not found in element %d", elem_index);
        return ESP_ERR_INVALID_STATE;
    }

    common.opcode = ESP_BLE_MESH_MODEL_OP_GEN_MOVE_SET_UNACK;
    common.model = model;
    common.ctx.net_idx = 0x0000;
    common.ctx.app_idx = 0x0000;
    common.ctx.addr = addr;
    common.ctx.send_ttl = 3;
    common.msg_timeout = 0;

    set_state.move_set.op_en = false;
    set_state.move_set.delta_level = move;
    set_state.move_set.tid = 0;

    err = esp_ble_mesh_generic_client_set_state(&common, &set_state);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send move set (err %d)", err);
        return err;
    }

    ESP_LOGI(TAG, "Sent move %d to addr 0x%04x", move, addr);
    return ESP_OK;
}

esp_err_t ble_mesh_client_get_default_transition_time(uint16_t addr, uint8_t elem_index)
{
    esp_ble_mesh_client_common_param_t common = {0};
    esp_ble_mesh_model_t *model = NULL;
    esp_err_t err;

    if (!is_provisioned) {
        ESP_LOGW(TAG, "Device not provisioned yet, cannot send messages");
        return ESP_ERR_INVALID_STATE;
    }

    for (uint8_t i = 0; i < composition->elements[elem_index].sig_model_count; i++) {
        if (composition->elements[elem_index].sig_models[i].model_id == ESP_BLE_MESH_MODEL_ID_GEN_DEF_TRANS_TIME_CLI) {
            model = &composition->elements[elem_index].sig_models[i];
            break;
        }
    }

    if (model == NULL) {
        ESP_LOGE(TAG, "Generic Default Transition Time Client model not found in element %d", elem_index);
        return ESP_ERR_INVALID_STATE;
    }

    common.opcode = ESP_BLE_MESH_MODEL_OP_GEN_DEF_TRANS_TIME_GET;
    common.model = model;
    common.ctx.net_idx = 0x0000;
    common.ctx.app_idx = 0x0000;
    common.ctx.addr = addr;
    common.ctx.send_ttl = 3;
    common.msg_timeout = 0;

    err = esp_ble_mesh_generic_client_get_state(&common, NULL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send default transition time get (err %d)", err);
        return err;
    }

    ESP_LOGI(TAG, "Sent default transition time get to addr 0x%04x", addr);
    return ESP_OK;
}

esp_err_t ble_mesh_client_set_default_transition_time(uint8_t transition_time, uint16_t addr, uint8_t elem_index)
{
    esp_ble_mesh_generic_client_set_state_t set_state = {0};
    esp_ble_mesh_client_common_param_t common = {0};
    esp_ble_mesh_model_t *model = NULL;
    esp_err_t err;
    
    if (!is_provisioned) {
        ESP_LOGW(TAG, "Device not provisioned yet, cannot send messages");
        return ESP_ERR_INVALID_STATE;
    }

    for (uint8_t i = 0; i < composition->elements[elem_index].sig_model_count; i++) {
        if (composition->elements[elem_index].sig_models[i].model_id == ESP_BLE_MESH_MODEL_ID_GEN_DEF_TRANS_TIME_CLI) {
            model = &composition->elements[elem_index].sig_models[i];
            break;
        }
    }

    if (model == NULL) {
        ESP_LOGE(TAG, "Generic Default Transition Time Client model not found in element %d", elem_index);
        return ESP_ERR_INVALID_STATE;
    }
    
    common.opcode = ESP_BLE_MESH_MODEL_OP_GEN_DEF_TRANS_TIME_SET_UNACK;
    common.model = model;
    common.ctx.net_idx = 0x0000;
    common.ctx.app_idx = 0x0000;
    common.ctx.addr = addr;
    common.ctx.send_ttl = 3;
    common.msg_timeout = 0;
    
    set_state.def_trans_time_set.trans_time = transition_time;
    
    err = esp_ble_mesh_generic_client_set_state(&common, &set_state);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send default transition time set (err %d)", err);
        return err;
    }

    ESP_LOGI(TAG, "Sent default transition time %d to addr 0x%04x", transition_time, addr);
    return ESP_OK;
}

esp_err_t ble_mesh_client_get_onpowerup(uint16_t addr, uint8_t elem_index)
{
    esp_ble_mesh_client_common_param_t common = {0};
    esp_ble_mesh_model_t *model = NULL;
    esp_err_t err;

    if (!is_provisioned) {
        ESP_LOGW(TAG, "Device not provisioned yet, cannot send messages");
        return ESP_ERR_INVALID_STATE;
    }

    for (uint8_t i = 0; i < composition->elements[elem_index].sig_model_count; i++) {
        if (composition->elements[elem_index].sig_models[i].model_id == ESP_BLE_MESH_MODEL_ID_GEN_POWER_ONOFF_CLI) {
            model = &composition->elements[elem_index].sig_models[i];
            break;
        }
    }

    if (model == NULL) {
        ESP_LOGE(TAG, "Generic Power OnOff Client model not found in element %d", elem_index);
        return ESP_ERR_INVALID_STATE;
    }

    common.opcode = ESP_BLE_MESH_MODEL_OP_GEN_ONPOWERUP_GET;
    common.model = model;
    common.ctx.net_idx = 0x0000;
    common.ctx.app_idx = 0x0000;
    common.ctx.addr = addr;
    common.ctx.send_ttl = 3;
    common.msg_timeout = 0;

    err = esp_ble_mesh_generic_client_get_state(&common, NULL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send power onoff get (err %d)", err);
        return err;
    }

    ESP_LOGI(TAG, "Sent power onoff get to addr 0x%04x", addr);
    return ESP_OK;
}

esp_err_t ble_mesh_client_set_onpowerup(uint8_t power, uint16_t addr, uint8_t elem_index)
{
    esp_ble_mesh_generic_client_set_state_t set_state = {0};
    esp_ble_mesh_client_common_param_t common = {0};
    esp_ble_mesh_model_t *model = NULL;
    esp_err_t err;

    if (!is_provisioned) {
        ESP_LOGW(TAG, "Device not provisioned yet, cannot send messages");
        return ESP_ERR_INVALID_STATE;
    }

    for (uint8_t i = 0; i < composition->elements[elem_index].sig_model_count; i++) {
        if (composition->elements[elem_index].sig_models[i].model_id == ESP_BLE_MESH_MODEL_ID_GEN_POWER_ONOFF_CLI) {
            model = &composition->elements[elem_index].sig_models[i];
            break;
        }
    }

    if (model == NULL) {
        ESP_LOGE(TAG, "Generic Power OnOff Client model not found in element %d", elem_index);
        return ESP_ERR_INVALID_STATE;
    }

    common.opcode = ESP_BLE_MESH_MODEL_OP_GEN_ONPOWERUP_SET_UNACK;
    common.model = model;
    common.ctx.net_idx = 0x0000;
    common.ctx.app_idx = 0x0000;
    common.ctx.addr = addr;
    common.ctx.send_ttl = 3;
    common.msg_timeout = 0;

    set_state.power_set.onpowerup = power;

    err = esp_ble_mesh_generic_client_set_state(&common, &set_state);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send power onoff set (err %d)", err);
        return err;
    }

    ESP_LOGI(TAG, "Sent power onoff %d to addr 0x%04x", power, addr);
    return ESP_OK;
}

esp_err_t ble_mesh_client_get_power_level(uint16_t addr, uint8_t elem_index)
{
    esp_ble_mesh_client_common_param_t common = {0};
    esp_ble_mesh_model_t *model = NULL;
    esp_err_t err;

    if (!is_provisioned) {
        ESP_LOGW(TAG, "Device not provisioned yet, cannot send messages");
        return ESP_ERR_INVALID_STATE;
    }

    for (uint8_t i = 0; i < composition->elements[elem_index].sig_model_count; i++) {
        if (composition->elements[elem_index].sig_models[i].model_id == ESP_BLE_MESH_MODEL_ID_GEN_POWER_LEVEL_CLI) {
            model = &composition->elements[elem_index].sig_models[i];
            break;
        }
    }

    if (model == NULL) {
        ESP_LOGE(TAG, "Generic Power Level Client model not found in element %d", elem_index);
        return ESP_ERR_INVALID_STATE;
    }

    common.opcode = ESP_BLE_MESH_MODEL_OP_GEN_POWER_LEVEL_GET;
    common.model = model;
    common.ctx.net_idx = 0x0000;
    common.ctx.app_idx = 0x0000;
    common.ctx.addr = addr;
    common.ctx.send_ttl = 3;
    common.msg_timeout = 0;

    err = esp_ble_mesh_generic_client_get_state(&common, NULL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send power level get (err %d)", err);
        return err;
    }

    ESP_LOGI(TAG, "Sent power level get to addr 0x%04x", addr);
    return ESP_OK;
}

esp_err_t ble_mesh_client_set_power_level(uint16_t power, uint16_t addr, uint8_t elem_index)
{
    esp_ble_mesh_generic_client_set_state_t set_state = {0};
    esp_ble_mesh_client_common_param_t common = {0};
    esp_ble_mesh_model_t *model = NULL;
    esp_err_t err;
    
    if (!is_provisioned) {
        ESP_LOGW(TAG, "Device not provisioned yet, cannot send messages");
        return ESP_ERR_INVALID_STATE;
    }

    for (uint8_t i = 0; i < composition->elements[elem_index].sig_model_count; i++) {
        if (composition->elements[elem_index].sig_models[i].model_id == ESP_BLE_MESH_MODEL_ID_GEN_POWER_LEVEL_CLI) {
            model = &composition->elements[elem_index].sig_models[i];
            break;
        }
    }

    if (model == NULL) {
        ESP_LOGE(TAG, "Generic Power Level Client model not found in element %d", elem_index);
        return ESP_ERR_INVALID_STATE;
    }
    
    common.opcode = ESP_BLE_MESH_MODEL_OP_GEN_POWER_LEVEL_SET_UNACK;
    common.model = model;
    common.ctx.net_idx = 0x0000;
    common.ctx.app_idx = 0x0000;
    common.ctx.addr = addr;
    common.ctx.send_ttl = 3;
    common.msg_timeout = 0;
    
    set_state.power_level_set.op_en = false;
    set_state.power_level_set.power = power;
    set_state.power_level_set.tid = 0;
    
    err = esp_ble_mesh_generic_client_set_state(&common, &set_state);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send power level set (err %d)", err);
        return err;
    }

    ESP_LOGI(TAG, "Sent power level %d to addr 0x%04x", power, addr);
    return ESP_OK;
}

esp_err_t ble_mesh_client_get_power_default(uint16_t addr, uint8_t elem_index)
{
    esp_ble_mesh_client_common_param_t common = {0};
    esp_ble_mesh_model_t *model = NULL;
    esp_err_t err;

    if (!is_provisioned) {
        ESP_LOGW(TAG, "Device not provisioned yet, cannot send messages");
        return ESP_ERR_INVALID_STATE;
    }

    for (uint8_t i = 0; i < composition->elements[elem_index].sig_model_count; i++) {
        if (composition->elements[elem_index].sig_models[i].model_id == ESP_BLE_MESH_MODEL_ID_GEN_POWER_LEVEL_CLI) {
            model = &composition->elements[elem_index].sig_models[i];
            break;
        }
    }

    if (model == NULL) {
        ESP_LOGE(TAG, "Generic Power Level Client model not found in element %d", elem_index);
        return ESP_ERR_INVALID_STATE;
    }

    common.opcode = ESP_BLE_MESH_MODEL_OP_GEN_POWER_DEFAULT_GET;
    common.model = model;
    common.ctx.net_idx = 0x0000;
    common.ctx.app_idx = 0x0000;
    common.ctx.addr = addr;
    common.ctx.send_ttl = 3;
    common.msg_timeout = 0;

    err = esp_ble_mesh_generic_client_get_state(&common, NULL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send power default get (err %d)", err);
        return err;
    }

    ESP_LOGI(TAG, "Sent power default get to addr 0x%04x", addr);
    return ESP_OK;
}

esp_err_t ble_mesh_client_set_power_default(uint16_t power_default, uint16_t addr, uint8_t elem_index)
{
    esp_ble_mesh_generic_client_set_state_t set_state = {0};
    esp_ble_mesh_client_common_param_t common = {0};
    esp_ble_mesh_model_t *model = NULL;
    esp_err_t err;

    if (!is_provisioned) {
        ESP_LOGW(TAG, "Device not provisioned yet, cannot send messages");
        return ESP_ERR_INVALID_STATE;
    }

    for (uint8_t i = 0; i < composition->elements[elem_index].sig_model_count; i++) {
        if (composition->elements[elem_index].sig_models[i].model_id == ESP_BLE_MESH_MODEL_ID_GEN_POWER_LEVEL_CLI) {
            model = &composition->elements[elem_index].sig_models[i];
            break;
        }
    }

    if (model == NULL) {
        ESP_LOGE(TAG, "Generic Power Level Client model not found in element %d", elem_index);
        return ESP_ERR_INVALID_STATE;
    }

    common.opcode = ESP_BLE_MESH_MODEL_OP_GEN_POWER_DEFAULT_SET_UNACK;
    common.model = model;
    common.ctx.net_idx = 0x0000;
    common.ctx.app_idx = 0x0000;
    common.ctx.addr = addr;
    common.ctx.send_ttl = 3;
    common.msg_timeout = 0;

    set_state.power_default_set.power = power_default;

    err = esp_ble_mesh_generic_client_set_state(&common, &set_state);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send power default set (err %d)", err);
        return err;
    }

    ESP_LOGI(TAG, "Sent power default %d to addr 0x%04x", power_default, addr);
    return ESP_OK;
}

esp_err_t ble_mesh_client_get_power_range(uint16_t addr, uint8_t elem_index)
{
    esp_ble_mesh_client_common_param_t common = {0};
    esp_ble_mesh_model_t *model = NULL;
    esp_err_t err;

    if (!is_provisioned) {
        ESP_LOGW(TAG, "Device not provisioned yet, cannot send messages");
        return ESP_ERR_INVALID_STATE;
    }

    for (uint8_t i = 0; i < composition->elements[elem_index].sig_model_count; i++) {
        if (composition->elements[elem_index].sig_models[i].model_id == ESP_BLE_MESH_MODEL_ID_GEN_POWER_LEVEL_CLI) {
            model = &composition->elements[elem_index].sig_models[i];
            break;
        }
    }

    if (model == NULL) {
        ESP_LOGE(TAG, "Generic Power Level Client model not found in element %d", elem_index);
        return ESP_ERR_INVALID_STATE;
    }

    common.opcode = ESP_BLE_MESH_MODEL_OP_GEN_POWER_RANGE_GET;
    common.model = model;
    common.ctx.net_idx = 0x0000;
    common.ctx.app_idx = 0x0000;
    common.ctx.addr = addr;
    common.ctx.send_ttl = 3;
    common.msg_timeout = 0;

    err = esp_ble_mesh_generic_client_get_state(&common, NULL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send power range get (err %d)", err);
        return err;
    }

    ESP_LOGI(TAG, "Sent power range get to addr 0x%04x", addr);
    return ESP_OK;
}

esp_err_t ble_mesh_client_set_power_range(uint16_t min, uint16_t max, uint16_t addr, uint8_t elem_index)
{
    esp_ble_mesh_generic_client_set_state_t set_state = {0};
    esp_ble_mesh_client_common_param_t common = {0};
    esp_ble_mesh_model_t *model = NULL;
    esp_err_t err;

    if (!is_provisioned) {
        ESP_LOGW(TAG, "Device not provisioned yet, cannot send messages");
        return ESP_ERR_INVALID_STATE;
    }

    for (uint8_t i = 0; i < composition->elements[elem_index].sig_model_count; i++) {
        if (composition->elements[elem_index].sig_models[i].model_id == ESP_BLE_MESH_MODEL_ID_GEN_POWER_LEVEL_CLI) {
            model = &composition->elements[elem_index].sig_models[i];
            break;
        }
    }

    if (model == NULL) {
        ESP_LOGE(TAG, "Generic Power Level Client model not found in element %d", elem_index);
        return ESP_ERR_INVALID_STATE;
    }

    common.opcode = ESP_BLE_MESH_MODEL_OP_GEN_POWER_RANGE_SET_UNACK;
    common.model = model;
    common.ctx.net_idx = 0x0000;
    common.ctx.app_idx = 0x0000;
    common.ctx.addr = addr;
    common.ctx.send_ttl = 3;
    common.msg_timeout = 0;

    set_state.power_range_set.range_min = min;
    set_state.power_range_set.range_max = max;

    err = esp_ble_mesh_generic_client_set_state(&common, &set_state);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send power range set (err %d)", err);
        return err;
    }

    ESP_LOGI(TAG, "Sent power range min %d and max %d to addr 0x%04x", min, max, addr);
    return ESP_OK;
}

esp_err_t ble_mesh_client_get_battery(uint16_t addr, uint8_t elem_index)
{
    esp_ble_mesh_client_common_param_t common = {0};
    esp_ble_mesh_model_t *model = NULL;
    esp_err_t err;
    
    if (!is_provisioned) {
        ESP_LOGW(TAG, "Device not provisioned yet, cannot send messages");
        return ESP_ERR_INVALID_STATE;
    }

    for (uint8_t i = 0; i < composition->elements[elem_index].sig_model_count; i++) {
        if (composition->elements[elem_index].sig_models[i].model_id == ESP_BLE_MESH_MODEL_ID_GEN_BATTERY_CLI) {
            model = &composition->elements[elem_index].sig_models[i];
            break;
        }
    }

    if (model == NULL) {
        ESP_LOGE(TAG, "Generic Battery Client model not found in element %d", elem_index);
        return ESP_ERR_INVALID_STATE;
    }

    common.opcode = ESP_BLE_MESH_MODEL_OP_GEN_BATTERY_GET;
    common.model = model;
    common.ctx.net_idx = 0x0000;
    common.ctx.app_idx = 0x0000;
    common.ctx.addr = addr;
    common.ctx.send_ttl = 3;
    common.msg_timeout = 0;
    
    err = esp_ble_mesh_generic_client_get_state(&common, NULL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send battery get (err %d)", err);
        return err;
    }

    ESP_LOGI(TAG, "Sent battery get to addr 0x%04x", addr);
    return ESP_OK;
}

esp_err_t ble_mesh_client_get_loc_global(uint16_t addr, uint8_t elem_index)
{
    esp_ble_mesh_client_common_param_t common = {0};
    esp_ble_mesh_model_t *model = NULL;
    esp_err_t err;

    if (!is_provisioned) {
        ESP_LOGW(TAG, "Device not provisioned yet, cannot send messages");
        return ESP_ERR_INVALID_STATE;
    }

    for (uint8_t i = 0; i < composition->elements[elem_index].sig_model_count; i++) {
        if (composition->elements[elem_index].sig_models[i].model_id == ESP_BLE_MESH_MODEL_ID_GEN_LOCATION_CLI) {
            model = &composition->elements[elem_index].sig_models[i];
            break;
        }
    }

    if (model == NULL) {
        ESP_LOGE(TAG, "Generic Location Client model not found in element %d", elem_index);
        return ESP_ERR_INVALID_STATE;
    }

    common.opcode = ESP_BLE_MESH_MODEL_OP_GEN_LOC_GLOBAL_GET;
    common.model = model;
    common.ctx.net_idx = 0x0000;
    common.ctx.app_idx = 0x0000;
    common.ctx.addr = addr;
    common.ctx.send_ttl = 3;
    common.msg_timeout = 0;

    err = esp_ble_mesh_generic_client_get_state(&common, NULL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send location global get (err %d)", err);
        return err;
    }

    ESP_LOGI(TAG, "Sent location global get to addr 0x%04x", addr);
    return ESP_OK;
}

esp_err_t ble_mesh_client_set_loc_global(int32_t latitude, int32_t longitude, int16_t altitude, uint16_t addr, uint8_t elem_index)
{
    esp_ble_mesh_generic_client_set_state_t set_state = {0};
    esp_ble_mesh_client_common_param_t common = {0};
    esp_ble_mesh_model_t *model = NULL;
    esp_err_t err;
    
    if (!is_provisioned) {
        ESP_LOGW(TAG, "Device not provisioned yet, cannot send messages");
        return ESP_ERR_INVALID_STATE;
    }

    for (uint8_t i = 0; i < composition->elements[elem_index].sig_model_count; i++) {
        if (composition->elements[elem_index].sig_models[i].model_id == ESP_BLE_MESH_MODEL_ID_GEN_LOCATION_CLI) {
            model = &composition->elements[elem_index].sig_models[i];
            break;
        }
    }

    if (model == NULL) {
        ESP_LOGE(TAG, "Generic Location Client model not found in element %d", elem_index);
        return ESP_ERR_INVALID_STATE;
    }
    
    common.opcode = ESP_BLE_MESH_MODEL_OP_GEN_LOC_GLOBAL_SET_UNACK;
    common.model = model;
    common.ctx.net_idx = 0x0000;
    common.ctx.app_idx = 0x0000;
    common.ctx.addr = addr;
    common.ctx.send_ttl = 3;
    common.msg_timeout = 0;
    
    set_state.loc_global_set.global_latitude = latitude;
    set_state.loc_global_set.global_longitude = longitude;
    set_state.loc_global_set.global_altitude = altitude;
    
    err = esp_ble_mesh_generic_client_set_state(&common, &set_state);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send location global set (err %d)", err);
        return err;
    }

    ESP_LOGI(TAG, "Sent global location (lat=%lu, lon=%lu, alt=%d) to addr 0x%04x",
                latitude, longitude, altitude, addr);
    return ESP_OK;
}

esp_err_t ble_mesh_client_get_loc_local(uint16_t addr, uint8_t elem_index)
{
    esp_ble_mesh_client_common_param_t common = {0};
    esp_ble_mesh_model_t *model = NULL;
    esp_err_t err;

    if (!is_provisioned) {
        ESP_LOGW(TAG, "Device not provisioned yet, cannot send messages");
        return ESP_ERR_INVALID_STATE;
    }

    for (uint8_t i = 0; i < composition->elements[elem_index].sig_model_count; i++) {
        if (composition->elements[elem_index].sig_models[i].model_id == ESP_BLE_MESH_MODEL_ID_GEN_LOCATION_CLI) {
            model = &composition->elements[elem_index].sig_models[i];
            break;
        }
    }

    if (model == NULL) {
        ESP_LOGE(TAG, "Generic Location Client model not found in element %d", elem_index);
        return ESP_ERR_INVALID_STATE;
    }

    common.opcode = ESP_BLE_MESH_MODEL_OP_GEN_LOC_LOCAL_GET;
    common.model = model;
    common.ctx.net_idx = 0x0000;
    common.ctx.app_idx = 0x0000;
    common.ctx.addr = addr;
    common.ctx.send_ttl = 3;
    common.msg_timeout = 0;

    err = esp_ble_mesh_generic_client_get_state(&common, NULL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send location local get (err %d)", err);
        return err;
    }

    ESP_LOGI(TAG, "Sent location local get to addr 0x%04x", addr);
    return ESP_OK;
}

esp_err_t ble_mesh_client_set_loc_local(int16_t north, int16_t east, int16_t altitude, uint8_t floor_number,
                                        uint16_t uncertainty, uint16_t addr, uint8_t elem_index)
{
    esp_ble_mesh_generic_client_set_state_t set_state = {0};
    esp_ble_mesh_client_common_param_t common = {0};
    esp_ble_mesh_model_t *model = NULL;
    esp_err_t err;

    if (!is_provisioned) {
        ESP_LOGW(TAG, "Device not provisioned yet, cannot send messages");
        return ESP_ERR_INVALID_STATE;
    }

    for (uint8_t i = 0; i < composition->elements[elem_index].sig_model_count; i++) {
        if (composition->elements[elem_index].sig_models[i].model_id == ESP_BLE_MESH_MODEL_ID_GEN_LOCATION_CLI) {
            model = &composition->elements[elem_index].sig_models[i];
            break;
        }
    }

    if (model == NULL) {
        ESP_LOGE(TAG, "Generic Location Client model not found in element %d", elem_index);
        return ESP_ERR_INVALID_STATE;
    }

    common.opcode = ESP_BLE_MESH_MODEL_OP_GEN_LOC_LOCAL_SET_UNACK;
    common.model = model;
    common.ctx.net_idx = 0x0000;
    common.ctx.app_idx = 0x0000;
    common.ctx.addr = addr;
    common.ctx.send_ttl = 3;
    common.msg_timeout = 0;

    set_state.loc_local_set.local_north = north;
    set_state.loc_local_set.local_east = east;
    set_state.loc_local_set.local_altitude = altitude;
    set_state.loc_local_set.floor_number = floor_number;
    set_state.loc_local_set.uncertainty = uncertainty;

    err = esp_ble_mesh_generic_client_set_state(&common, &set_state);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send location local set (err %d)", err);
        return err;
    }

    ESP_LOGI(TAG, "Sent local location (north=%d, east=%d, alt=%d, floor=%d, unc=%d) to addr 0x%04x",
                north, east, altitude, floor_number, uncertainty, addr);
    return ESP_OK;
}

esp_err_t ble_mesh_client_get_user_properties(uint16_t addr, uint8_t elem_index)
{
    esp_ble_mesh_client_common_param_t common = {0};
    esp_ble_mesh_model_t *model = NULL;
    esp_err_t err;

    if (!is_provisioned) {
        ESP_LOGW(TAG, "Device not provisioned yet, cannot send messages");
        return ESP_ERR_INVALID_STATE;
    }

    for (uint8_t i = 0; i < composition->elements[elem_index].sig_model_count; i++) {
        if (composition->elements[elem_index].sig_models[i].model_id == ESP_BLE_MESH_MODEL_ID_GEN_PROP_CLI) {
            model = &composition->elements[elem_index].sig_models[i];
            break;
        }
    }

    if (model == NULL) {
        ESP_LOGE(TAG, "Generic Prop Client model not found in element %d", elem_index);
        return ESP_ERR_INVALID_STATE;
    }

    common.opcode = ESP_BLE_MESH_MODEL_OP_GEN_USER_PROPERTIES_GET;
    common.model = model;
    common.ctx.net_idx = 0x0000;
    common.ctx.app_idx = 0x0000;
    common.ctx.addr = addr;
    common.ctx.send_ttl = 3;
    common.msg_timeout = 0;

    err = esp_ble_mesh_generic_client_get_state(&common, NULL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send user props get (err %d)", err);
        return err;
    }

    ESP_LOGI(TAG, "Sent user props get to addr 0x%04x", addr);
    return ESP_OK;
}

esp_err_t ble_mesh_client_get_user_property(uint16_t property_id, uint16_t addr, uint8_t elem_index)
{
    esp_ble_mesh_generic_client_get_state_t get_state = {0};
    esp_ble_mesh_client_common_param_t common = {0};
    esp_ble_mesh_model_t *model = NULL;
    esp_err_t err;

    if (!is_provisioned) {
        ESP_LOGW(TAG, "Device not provisioned yet, cannot send messages");
        return ESP_ERR_INVALID_STATE;
    }

    for (uint8_t i = 0; i < composition->elements[elem_index].sig_model_count; i++) {
        if (composition->elements[elem_index].sig_models[i].model_id == ESP_BLE_MESH_MODEL_ID_GEN_PROP_CLI) {
            model = &composition->elements[elem_index].sig_models[i];
            break;
        }
    }

    if (model == NULL) {
        ESP_LOGE(TAG, "Generic Prop Client model not found in element %d", elem_index);
        return ESP_ERR_INVALID_STATE;
    }

    common.opcode = ESP_BLE_MESH_MODEL_OP_GEN_USER_PROPERTY_GET;
    common.model = model;
    common.ctx.net_idx = 0x0000;
    common.ctx.app_idx = 0x0000;
    common.ctx.addr = addr;
    common.ctx.send_ttl = 3;
    common.msg_timeout = 0;

    get_state.user_property_get.property_id = property_id;

    err = esp_ble_mesh_generic_client_get_state(&common, &get_state);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send user prop get (err %d)", err);
        return err;
    }

    ESP_LOGI(TAG, "Sent user prop get 0x%04x to addr 0x%04x", property_id, addr);
    return ESP_OK;
}

esp_err_t ble_mesh_client_set_user_property(uint16_t property_id, uint8_t *property_value,
                                    uint16_t property_value_len, uint16_t addr, uint8_t elem_index)
{
    esp_ble_mesh_generic_client_set_state_t set_state = {0};
    esp_ble_mesh_client_common_param_t common = {0};
    esp_ble_mesh_model_t *model = NULL;
    esp_err_t err;
    
    if (!is_provisioned) {
        ESP_LOGW(TAG, "Device not provisioned yet, cannot send messages");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (property_value_len > 379) {  // Max property value length
        ESP_LOGE(TAG, "Property value too long (max 379 bytes)");
        return ESP_ERR_INVALID_ARG;
    }

    for (uint8_t i = 0; i < composition->elements[elem_index].sig_model_count; i++) {
        if (composition->elements[elem_index].sig_models[i].model_id == ESP_BLE_MESH_MODEL_ID_GEN_PROP_CLI) {
            model = &composition->elements[elem_index].sig_models[i];
            break;
        }
    }

    if (model == NULL) {
        ESP_LOGE(TAG, "Generic Prop Client model not found in element %d", elem_index);
        return ESP_ERR_INVALID_STATE;
    }
    
    common.opcode = ESP_BLE_MESH_MODEL_OP_GEN_USER_PROPERTY_SET_UNACK;
    common.model = model;  // Generic Property Client
    common.ctx.net_idx = 0x0000;
    common.ctx.app_idx = 0x0000;
    common.ctx.addr = addr;
    common.ctx.send_ttl = 3;
    common.msg_timeout = 0;
    
    set_state.user_property_set.property_id = property_id;
    set_state.user_property_set.property_value->data = property_value;

    err = esp_ble_mesh_generic_client_set_state(&common, &set_state);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send user property set (err %d)", err);
        return err;
    }

    ESP_LOGI(TAG, "Sent user property 0x%04x (len=%d) to addr 0x%04x",
                property_id, property_value_len, addr);
    return ESP_OK;
}

esp_err_t ble_mesh_client_get_admin_properties(uint16_t addr, uint8_t elem_index)
{
    esp_ble_mesh_client_common_param_t common = {0};
    esp_ble_mesh_model_t *model = NULL;
    esp_err_t err;

    if (!is_provisioned) {
        ESP_LOGW(TAG, "Device not provisioned yet, cannot send messages");
        return ESP_ERR_INVALID_STATE;
    }

    for (uint8_t i = 0; i < composition->elements[elem_index].sig_model_count; i++) {
        if (composition->elements[elem_index].sig_models[i].model_id == ESP_BLE_MESH_MODEL_ID_GEN_PROP_CLI) {
            model = &composition->elements[elem_index].sig_models[i];
            break;
        }
    }

    if (model == NULL) {
        ESP_LOGE(TAG, "Generic Prop Client model not found in element %d", elem_index);
        return ESP_ERR_INVALID_STATE;
    }

    common.opcode = ESP_BLE_MESH_MODEL_OP_GEN_ADMIN_PROPERTIES_GET;
    common.model = model;
    common.ctx.net_idx = 0x0000;
    common.ctx.app_idx = 0x0000;
    common.ctx.addr = addr;
    common.ctx.send_ttl = 3;
    common.msg_timeout = 0;

    err = esp_ble_mesh_generic_client_get_state(&common, NULL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send admin props get (err %d)", err);
        return err;
    }

    ESP_LOGI(TAG, "Sent admin props get to addr 0x%04x", addr);
    return ESP_OK;
}

esp_err_t ble_mesh_client_get_admin_property(uint16_t property_id, uint16_t addr, uint8_t elem_index)
{
    esp_ble_mesh_generic_client_get_state_t get_state = {0};
    esp_ble_mesh_client_common_param_t common = {0};
    esp_ble_mesh_model_t *model = NULL;
    esp_err_t err;

    if (!is_provisioned) {
        ESP_LOGW(TAG, "Device not provisioned yet, cannot send messages");
        return ESP_ERR_INVALID_STATE;
    }

    for (uint8_t i = 0; i < composition->elements[elem_index].sig_model_count; i++) {
        if (composition->elements[elem_index].sig_models[i].model_id == ESP_BLE_MESH_MODEL_ID_GEN_PROP_CLI) {
            model = &composition->elements[elem_index].sig_models[i];
            break;
        }
    }

    if (model == NULL) {
        ESP_LOGE(TAG, "Generic Prop Client model not found in element %d", elem_index);
        return ESP_ERR_INVALID_STATE;
    }

    common.opcode = ESP_BLE_MESH_MODEL_OP_GEN_ADMIN_PROPERTY_GET;
    common.model = model;
    common.ctx.net_idx = 0x0000;
    common.ctx.app_idx = 0x0000;
    common.ctx.addr = addr;
    common.ctx.send_ttl = 3;
    common.msg_timeout = 0;

    get_state.admin_property_get.property_id = property_id;

    err = esp_ble_mesh_generic_client_get_state(&common, &get_state);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send admin prop get (err %d)", err);
        return err;
    }

    ESP_LOGI(TAG, "Sent admin prop get 0x%04x to addr 0x%04x", property_id, addr);
    return ESP_OK;
}

esp_err_t ble_mesh_client_set_admin_property(uint16_t property_id, uint8_t admin_access, uint8_t *property_value,
                                    uint16_t property_value_len, uint16_t addr, uint8_t elem_index)
{
    esp_ble_mesh_generic_client_set_state_t set_state = {0};
    esp_ble_mesh_client_common_param_t common = {0};
    esp_ble_mesh_model_t *model = NULL;
    esp_err_t err;

    if (!is_provisioned) {
        ESP_LOGW(TAG, "Device not provisioned yet, cannot send messages");
        return ESP_ERR_INVALID_STATE;
    }

    if (property_value_len > 379) {  // Max property value length
        ESP_LOGE(TAG, "Property value too long (max 379 bytes)");
        return ESP_ERR_INVALID_ARG;
    }

    for (uint8_t i = 0; i < composition->elements[elem_index].sig_model_count; i++) {
        if (composition->elements[elem_index].sig_models[i].model_id == ESP_BLE_MESH_MODEL_ID_GEN_PROP_CLI) {
            model = &composition->elements[elem_index].sig_models[i];
            break;
        }
    }

    if (model == NULL) {
        ESP_LOGE(TAG, "Generic Prop Client model not found in element %d", elem_index);
        return ESP_ERR_INVALID_STATE;
    }

    common.opcode = ESP_BLE_MESH_MODEL_OP_GEN_ADMIN_PROPERTY_SET_UNACK;
    common.model = model;  // Generic Property Client
    common.ctx.net_idx = 0x0000;
    common.ctx.app_idx = 0x0000;
    common.ctx.addr = addr;
    common.ctx.send_ttl = 3;
    common.msg_timeout = 0;

    set_state.admin_property_set.property_id = property_id;
    set_state.admin_property_set.user_access = admin_access;
    set_state.admin_property_set.property_value->data = property_value;

    err = esp_ble_mesh_generic_client_set_state(&common, &set_state);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send admin property set (err %d)", err);
        return err;
    }

    ESP_LOGI(TAG, "Sent admin property 0x%04x (access=%d, len=%d) to addr 0x%04x",
                property_id, property_value_len, addr);
    return ESP_OK;
}

esp_err_t ble_mesh_client_get_manu_properties(uint16_t addr, uint8_t elem_index)
{
    esp_ble_mesh_client_common_param_t common = {0};
    esp_ble_mesh_model_t *model = NULL;
    esp_err_t err;

    if (!is_provisioned) {
        ESP_LOGW(TAG, "Device not provisioned yet, cannot send messages");
        return ESP_ERR_INVALID_STATE;
    }

    for (uint8_t i = 0; i < composition->elements[elem_index].sig_model_count; i++) {
        if (composition->elements[elem_index].sig_models[i].model_id == ESP_BLE_MESH_MODEL_ID_GEN_PROP_CLI) {
            model = &composition->elements[elem_index].sig_models[i];
            break;
        }
    }

    if (model == NULL) {
        ESP_LOGE(TAG, "Generic Prop Client model not found in element %d", elem_index);
        return ESP_ERR_INVALID_STATE;
    }

    common.opcode = ESP_BLE_MESH_MODEL_OP_GEN_MANUFACTURER_PROPERTIES_GET;
    common.model = model;
    common.ctx.net_idx = 0x0000;
    common.ctx.app_idx = 0x0000;
    common.ctx.addr = addr;
    common.ctx.send_ttl = 3;
    common.msg_timeout = 0;

    err = esp_ble_mesh_generic_client_get_state(&common, NULL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send manu props get (err %d)", err);
        return err;
    }

    ESP_LOGI(TAG, "Sent manu props get to addr 0x%04x", addr);
    return ESP_OK;
}

esp_err_t ble_mesh_client_get_manu_property(uint16_t property_id, uint16_t addr, uint8_t elem_index)
{
    esp_ble_mesh_generic_client_get_state_t get_state = {0};
    esp_ble_mesh_client_common_param_t common = {0};
    esp_ble_mesh_model_t *model = NULL;
    esp_err_t err;

    if (!is_provisioned) {
        ESP_LOGW(TAG, "Device not provisioned yet, cannot send messages");
        return ESP_ERR_INVALID_STATE;
    }

    for (uint8_t i = 0; i < composition->elements[elem_index].sig_model_count; i++) {
        if (composition->elements[elem_index].sig_models[i].model_id == ESP_BLE_MESH_MODEL_ID_GEN_PROP_CLI) {
            model = &composition->elements[elem_index].sig_models[i];
            break;
        }
    }

    if (model == NULL) {
        ESP_LOGE(TAG, "Generic Prop Client model not found in element %d", elem_index);
        return ESP_ERR_INVALID_STATE;
    }

    common.opcode = ESP_BLE_MESH_MODEL_OP_GEN_MANUFACTURER_PROPERTIES_GET;
    common.model = model;
    common.ctx.net_idx = 0x0000;
    common.ctx.app_idx = 0x0000;
    common.ctx.addr = addr;
    common.ctx.send_ttl = 3;
    common.msg_timeout = 0;

    get_state.user_property_get.property_id = property_id;

    err = esp_ble_mesh_generic_client_get_state(&common, &get_state);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send manu prop get (err %d)", err);
        return err;
    }

    ESP_LOGI(TAG, "Sent manu prop get 0x%04x to addr 0x%04x", property_id, addr);
    return ESP_OK;
}

esp_err_t ble_mesh_client_set_manu_property(uint16_t property_id, uint8_t manu_access, uint16_t addr, uint8_t elem_index)
{
    esp_ble_mesh_generic_client_set_state_t set_state = {0};
    esp_ble_mesh_client_common_param_t common = {0};
    esp_ble_mesh_model_t *model = NULL;
    esp_err_t err;

    if (!is_provisioned) {
        ESP_LOGW(TAG, "Device not provisioned yet, cannot send messages");
        return ESP_ERR_INVALID_STATE;
    }

    for (uint8_t i = 0; i < composition->elements[elem_index].sig_model_count; i++) {
        if (composition->elements[elem_index].sig_models[i].model_id == ESP_BLE_MESH_MODEL_ID_GEN_PROP_CLI) {
            model = &composition->elements[elem_index].sig_models[i];
            break;
        }
    }

    if (model == NULL) {
        ESP_LOGE(TAG, "Generic Prop Client model not found in element %d", elem_index);
        return ESP_ERR_INVALID_STATE;
    }

    common.opcode = ESP_BLE_MESH_MODEL_OP_GEN_MANUFACTURER_PROPERTY_SET_UNACK;
    common.model = model;  // Generic Property Client
    common.ctx.net_idx = 0x0000;
    common.ctx.app_idx = 0x0000;
    common.ctx.addr = addr;
    common.ctx.send_ttl = 3;
    common.msg_timeout = 0;

    set_state.manufacturer_property_set.property_id = property_id;
    set_state.manufacturer_property_set.user_access = manu_access;

    err = esp_ble_mesh_generic_client_set_state(&common, &set_state);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send manu property set (err %d)", err);
        return err;
    }

    ESP_LOGI(TAG, "Sent manu property 0x%04x (access=%d) to addr 0x%04x",
                property_id, manu_access, addr);
    return ESP_OK;
}

esp_err_t ble_mesh_client_get_client_properties(uint16_t property_id, uint16_t addr, uint8_t elem_index)
{
    esp_ble_mesh_generic_client_get_state_t get_state = {0};
    esp_ble_mesh_client_common_param_t common = {0};
    esp_ble_mesh_model_t *model = NULL;
    esp_err_t err;

    if (!is_provisioned) {
        ESP_LOGW(TAG, "Device not provisioned yet, cannot send messages");
        return ESP_ERR_INVALID_STATE;
    }

    for (uint8_t i = 0; i < composition->elements[elem_index].sig_model_count; i++) {
        if (composition->elements[elem_index].sig_models[i].model_id == ESP_BLE_MESH_MODEL_ID_GEN_PROP_CLI) {
            model = &composition->elements[elem_index].sig_models[i];
            break;
        }
    }

    if (model == NULL) {
        ESP_LOGE(TAG, "Generic Prop Client model not found in element %d", elem_index);
        return ESP_ERR_INVALID_STATE;
    }

    common.opcode = ESP_BLE_MESH_MODEL_OP_GEN_CLIENT_PROPERTIES_GET;
    common.model = model;
    common.ctx.net_idx = 0x0000;
    common.ctx.app_idx = 0x0000;
    common.ctx.addr = addr;
    common.ctx.send_ttl = 3;
    common.msg_timeout = 0;

    get_state.client_properties_get.property_id = property_id;

    err = esp_ble_mesh_generic_client_get_state(&common, &get_state);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send client props get (err %d)", err);
        return err;
    }

    ESP_LOGI(TAG, "Sent client props get to addr 0x%04x", addr);
    return ESP_OK;
}

esp_err_t ble_mesh_client_init(void)
{
    ESP_LOGI(TAG, "Initializing...");
    esp_err_t err;

    if (composition->element_count == 0) {
        ESP_LOGE(TAG, "Composition data is empty. Please define at least one element with models.");
        return ESP_ERR_INVALID_STATE;
    }
    
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
    
    err = esp_ble_mesh_init(&prov, composition);
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
