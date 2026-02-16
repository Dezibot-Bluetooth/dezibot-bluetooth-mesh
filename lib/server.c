#include "server.h"
#include "bluetooth.h"
#include "common.h"

#define TAG "BLE_MESH_SERVER"
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
    .output_size = 0,
    .output_actions = 0,
};

static void handle_gen_onoff_msg(esp_ble_mesh_model_t *model,
                                         esp_ble_mesh_msg_ctx_t *ctx,
                                         esp_ble_mesh_server_recv_gen_onoff_set_t *set)
{
    esp_ble_mesh_gen_onoff_srv_t *srv = model->user_data;

    switch (ctx->recv_op) {
        case ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_GET:
            esp_ble_mesh_server_model_send_msg(model, ctx,
                ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_STATUS, sizeof(srv->state.onoff), &srv->state.onoff);
            break;
        case ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_SET:
        case ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_SET_UNACK:
            srv->state.onoff = set->onoff;
            if (ctx->recv_op == ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_SET) {
                esp_ble_mesh_server_model_send_msg(model, ctx,
                    ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_STATUS, sizeof(srv->state.onoff), &srv->state.onoff);
            }
            esp_ble_mesh_model_publish(model, ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_STATUS,
                sizeof(srv->state.onoff), &srv->state.onoff, ROLE_NODE);
            break;
        default:
            break;
    }
}

static void mesh_generic_server_cb(esp_ble_mesh_generic_server_cb_event_t event,
                                   esp_ble_mesh_generic_server_cb_param_t *param)
{
    switch (event) {
        case ESP_BLE_MESH_GENERIC_SERVER_STATE_CHANGE_EVT:
            ESP_LOGI(TAG, "Generic server state changed");

            switch (param->ctx.recv_op)
            {
                case ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_SET:
                case ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_SET_UNACK:
                    uint8_t onoff_state = param->value.state_change.onoff_set.onoff;
                    ESP_LOGI(TAG, "OnOff state changed to %d", onoff_state);
                    break;
                case ESP_BLE_MESH_MODEL_OP_GEN_LEVEL_SET:
                case ESP_BLE_MESH_MODEL_OP_GEN_LEVEL_SET_UNACK:
                    int16_t level = param->value.state_change.level_set.level;
                    ESP_LOGI(TAG, "Level state changed to %d", level);
                    break;
                case ESP_BLE_MESH_MODEL_OP_GEN_DELTA_SET:
                case ESP_BLE_MESH_MODEL_OP_GEN_DELTA_SET_UNACK:
                    int16_t delta = param->value.state_change.delta_set.level;
                    ESP_LOGI(TAG, "Delta state changed by %d", delta);
                    break;
                case ESP_BLE_MESH_MODEL_OP_GEN_MOVE_SET:
                case ESP_BLE_MESH_MODEL_OP_GEN_MOVE_SET_UNACK:
                    int16_t move_level = param->value.state_change.move_set.level;
                    ESP_LOGI(TAG, "Move level changed to %d", move_level);
                    break;
                case ESP_BLE_MESH_MODEL_OP_GEN_DEF_TRANS_TIME_SET:
                case ESP_BLE_MESH_MODEL_OP_GEN_DEF_TRANS_TIME_SET_UNACK:
                    uint8_t trans_time = param->value.state_change.def_trans_time_set.trans_time;
                    ESP_LOGI(TAG, "Default Transition Time changed to %d", trans_time);
                    break;
                case ESP_BLE_MESH_MODEL_OP_GEN_ONPOWERUP_SET:
                case ESP_BLE_MESH_MODEL_OP_GEN_ONPOWERUP_SET_UNACK:
                    uint8_t onpowerup = param->value.state_change.onpowerup_set.onpowerup;
                    ESP_LOGI(TAG, "On Power Up state changed to %d", onpowerup);
                    break;
                case ESP_BLE_MESH_MODEL_OP_GEN_POWER_DEFAULT_SET:
                case ESP_BLE_MESH_MODEL_OP_GEN_POWER_DEFAULT_SET_UNACK:
                    uint16_t power_default = param->value.state_change.power_default_set.power;
                    ESP_LOGI(TAG, "Power Default changed to %d", power_default);
                    break;
                case ESP_BLE_MESH_MODEL_OP_GEN_POWER_LEVEL_SET:
                case ESP_BLE_MESH_MODEL_OP_GEN_POWER_LEVEL_SET_UNACK:
                    uint16_t power_level = param->value.state_change.power_level_set.power;
                    ESP_LOGI(TAG, "Power Level changed to %d", power_level);
                    break;
                case ESP_BLE_MESH_MODEL_OP_GEN_POWER_RANGE_SET:
                case ESP_BLE_MESH_MODEL_OP_GEN_POWER_RANGE_SET_UNACK:
                    uint16_t power_range_min = param->value.state_change.power_range_set.range_min;
                    uint16_t power_range_max = param->value.state_change.power_range_set.range_max;
                    ESP_LOGI(TAG, "Power Range changed to min: %d, max: %d", power_range_min, power_range_max);
                    break;
                case ESP_BLE_MESH_MODEL_OP_GEN_LOC_GLOBAL_SET:
                case ESP_BLE_MESH_MODEL_OP_GEN_LOC_GLOBAL_SET_UNACK:
                    int32_t latitude = param->value.state_change.loc_global_set.latitude;
                    int32_t longitude = param->value.state_change.loc_global_set.longitude;
                    int32_t altitude = param->value.state_change.loc_global_set.altitude;
                    ESP_LOGI(TAG, "Global Location changed to lat: %d, long: %d, alt: %d", latitude, longitude, altitude);
                    break;
                case ESP_BLE_MESH_MODEL_OP_GEN_LOC_LOCAL_SET:
                case ESP_BLE_MESH_MODEL_OP_GEN_LOC_LOCAL_SET_UNACK:
                    uint16_t north = param->value.state_change.loc_local_set.north;
                    uint16_t east = param->value.state_change.loc_local_set.east;
                    int16_t altitude_local = param->value.state_change.loc_local_set.altitude;
                    uint8_t floor_number = param->value.state_change.loc_local_set.floor_number;
                    uint16_t uncertainty = param->value.state_change.loc_local_set.uncertainty;
                    ESP_LOGI(TAG, "Local Location changed to north: %d, east: %d, alt: %d, floor: %d, uncertainty: %d",
                        north, east, altitude_local, floor_number, uncertainty);
                    break;
                case ESP_BLE_MESH_MODEL_OP_GEN_ADMIN_PROPERTY_SET:
                case ESP_BLE_MESH_MODEL_OP_GEN_ADMIN_PROPERTY_SET_UNACK:
                    uint16_t admin_prop_id = param->value.state_change.admin_property_set.id;
                    uint8_t admin_prop_access = param->value.state_change.admin_property_set.access;
                    ESP_LOGI(TAG, "Admin Property changed - ID: %d, Access: %d", admin_prop_id, admin_prop_access);
                    break;
                case ESP_BLE_MESH_MODEL_OP_GEN_MANUFACTURER_PROPERTY_SET:
                case ESP_BLE_MESH_MODEL_OP_GEN_MANUFACTURER_PROPERTY_SET_UNACK:
                    uint16_t manu_prop_id = param->value.state_change.manu_property_set.id;
                    uint8_t manu_prop_access = param->value.state_change.manu_property_set.access;
                    ESP_LOGI(TAG, "Manufacturer Property changed - ID: %d, Access: %d", manu_prop_id, manu_prop_access);
                    break;
                case ESP_BLE_MESH_MODEL_OP_GEN_USER_PROPERTY_SET:
                case ESP_BLE_MESH_MODEL_OP_GEN_USER_PROPERTY_SET_UNACK:
                    uint16_t user_prop_id = param->value.state_change.user_property_set.id;
                    ESP_LOGI(TAG, "User Property changed - ID: %d", user_prop_id);
                    break;
                default:
                    break;
            }

            break;
        case ESP_BLE_MESH_GENERIC_SERVER_RECV_GET_MSG_EVT:
            ESP_LOGI(TAG, "ESP_BLE_MESH_GENERIC_SERVER_RECV_GET_MSG_EVT");

            switch (param->ctx.recv_op)
            {
                case ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_GET:
                    const esp_ble_mesh_gen_onoff_srv_t *srv = param->model->user_data;
                    ESP_LOGI(TAG, "onoff %d", srv->state.onoff);
                    handle_gen_onoff_msg(param->model, &param->ctx, NULL);
                    break;
                case ESP_BLE_MESH_MODEL_OP_GEN_LEVEL_GET:
                    const esp_ble_mesh_gen_level_srv_t *level_srv = param->model->user_data;
                    ESP_LOGI(TAG, "level %d", level_srv->state.level);
                    break;
                case ESP_BLE_MESH_MODEL_OP_GEN_DEF_TRANS_TIME_GET:
                    const esp_ble_mesh_gen_def_trans_time_srv_t *def_trans_time = param->model->user_data;
                    ESP_LOGI(TAG, "default transition time %d", def_trans_time->state.trans_time);
                    break;
                case ESP_BLE_MESH_MODEL_OP_GEN_ONPOWERUP_GET:
                    const esp_ble_mesh_gen_power_onoff_srv_t *power_onoff_srv = param->model->user_data;
                    ESP_LOGI(TAG, "on power up %d", power_onoff_srv->state->onpowerup);
                    break;
                case ESP_BLE_MESH_MODEL_OP_GEN_POWER_DEFAULT_GET:
                    const esp_ble_mesh_gen_power_level_srv_t *power_default_srv = param->model->user_data;
                    ESP_LOGI(TAG, "power default %d", power_default_srv->state->power_default);
                    break;
                case ESP_BLE_MESH_MODEL_OP_GEN_POWER_LEVEL_GET:
                    const esp_ble_mesh_gen_power_level_srv_t *power_level_srv = param->model->user_data;
                    ESP_LOGI(TAG, "power level %d", power_level_srv->state->power_actual);
                    break;
                case ESP_BLE_MESH_MODEL_OP_GEN_POWER_LAST_GET:
                    const esp_ble_mesh_gen_power_level_srv_t *power_last_srv = param->model->user_data;
                    ESP_LOGI(TAG, "power last %d", power_last_srv->state->power_last);
                    break;
                case ESP_BLE_MESH_MODEL_OP_GEN_POWER_RANGE_GET:
                    const esp_ble_mesh_gen_power_level_srv_t *power_range_srv = param->model->user_data;
                    ESP_LOGI(TAG, "power range min %d, max %d", power_range_srv->state->power_range_min, power_range_srv->state->power_range_max);
                    break;
                case ESP_BLE_MESH_MODEL_OP_GEN_BATTERY_GET:
                    const esp_ble_mesh_gen_battery_srv_t *battery_srv = param->model->user_data;
                    ESP_LOGI(TAG, "battery level %d, time to discharge %d, time to charge %d, flags %d",
                        battery_srv->state.battery_level, battery_srv->state.time_to_discharge,
                        battery_srv->state.time_to_charge, battery_srv->state.battery_flags);
                    break;
                case ESP_BLE_MESH_MODEL_OP_GEN_LOC_GLOBAL_GET:
                    const esp_ble_mesh_gen_location_srv_t *loc_global_srv = param->model->user_data;
                    ESP_LOGI(TAG, "global location lat %d, long %d, alt %d",
                        loc_global_srv->state->global_latitude, loc_global_srv->state->global_longitude, loc_global_srv->state->global_altitude);
                    break;
                case ESP_BLE_MESH_MODEL_OP_GEN_LOC_LOCAL_GET:
                    const esp_ble_mesh_gen_location_srv_t *loc_local_srv = param->model->user_data;
                    ESP_LOGI(TAG, "local location north %d, east %d, alt %d, floor %d, uncertainty %d",
                        loc_local_srv->state->local_north, loc_local_srv->state->local_east, loc_local_srv->state->local_altitude,
                        loc_local_srv->state->floor_number, loc_local_srv->state->uncertainty);
                    break;
                case ESP_BLE_MESH_MODEL_OP_GEN_ADMIN_PROPERTIES_GET:
                    const esp_ble_mesh_gen_admin_prop_srv_t *admin_srv = param->model->user_data;
                    ESP_LOGI(TAG, "admin properties count %d", admin_srv->property_count);
                    break;
                case ESP_BLE_MESH_MODEL_OP_GEN_ADMIN_PROPERTY_GET:
                    const esp_ble_mesh_gen_admin_prop_srv_t *admin_prop_srv = param->model->user_data;
                    ESP_LOGI(TAG, "admin property id %d", admin_prop_srv->properties->id);
                    break;
                case ESP_BLE_MESH_MODEL_OP_GEN_MANUFACTURER_PROPERTIES_GET:
                    const esp_ble_mesh_gen_manu_prop_srv_t *manu_srv = param->model->user_data;
                    ESP_LOGI(TAG, "manufacturer properties count %d", manu_srv->property_count);
                    break;
                case ESP_BLE_MESH_MODEL_OP_GEN_MANUFACTURER_PROPERTY_GET:
                    const esp_ble_mesh_gen_manu_prop_srv_t *manu_prop_srv = param->model->user_data;
                    ESP_LOGI(TAG, "manufacturer property id %d", manu_prop_srv->properties->id);
                    break;
                case ESP_BLE_MESH_MODEL_OP_GEN_USER_PROPERTIES_GET:
                    const esp_ble_mesh_gen_user_prop_srv_t *user_srv = param->model->user_data;
                    ESP_LOGI(TAG, "user properties count %d", user_srv->property_count);
                    break;
                case ESP_BLE_MESH_MODEL_OP_GEN_USER_PROPERTY_GET:
                    const esp_ble_mesh_gen_user_prop_srv_t *user_prop_srv = param->model->user_data;
                    ESP_LOGI(TAG, "user property id %d", user_prop_srv->properties->id);
                     break;
                default:
                    break;
            }

            break;
        case ESP_BLE_MESH_GENERIC_SERVER_RECV_SET_MSG_EVT:
            ESP_LOGI(TAG, "ESP_BLE_MESH_GENERIC_SERVER_RECV_SET_MSG_EVT");

            switch (param->ctx.recv_op)
            {
                case ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_SET:
                case ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_SET_UNACK:
                    ESP_LOGI(TAG, "onoff %d, tid %d", param->value.set.onoff.onoff, param->value.set.onoff.tid);
                    handle_gen_onoff_msg(param->model, &param->ctx, &param->value.set.onoff);
                    break;
                case ESP_BLE_MESH_MODEL_OP_GEN_LEVEL_SET:
                case ESP_BLE_MESH_MODEL_OP_GEN_LEVEL_SET_UNACK:
                    ESP_LOGI(TAG, "level %d, tid %d", param->value.set.level.level, param->value.set.level.tid);
                    break;
                case ESP_BLE_MESH_MODEL_OP_GEN_DELTA_SET:
                case ESP_BLE_MESH_MODEL_OP_GEN_DELTA_SET_UNACK:
                    ESP_LOGI(TAG, "delta %d, tid %d", param->value.set.delta.delta_level, param->value.set.delta.tid);
                    break;
                case ESP_BLE_MESH_MODEL_OP_GEN_MOVE_SET:
                case ESP_BLE_MESH_MODEL_OP_GEN_MOVE_SET_UNACK:
                    ESP_LOGI(TAG, "move level %d, tid %d", param->value.set.move.delta_level, param->value.set.move.tid);
                    break;
                case ESP_BLE_MESH_MODEL_OP_GEN_DEF_TRANS_TIME_SET:
                case ESP_BLE_MESH_MODEL_OP_GEN_DEF_TRANS_TIME_SET_UNACK:
                    ESP_LOGI(TAG, "default transition time %d", param->value.set.def_trans_time.trans_time);
                    break;
                case ESP_BLE_MESH_MODEL_OP_GEN_ONPOWERUP_SET:
                case ESP_BLE_MESH_MODEL_OP_GEN_ONPOWERUP_SET_UNACK:
                    ESP_LOGI(TAG, "on power up %d", param->value.set.onpowerup.onpowerup);
                    break;
                case ESP_BLE_MESH_MODEL_OP_GEN_POWER_DEFAULT_SET:
                case ESP_BLE_MESH_MODEL_OP_GEN_POWER_DEFAULT_SET_UNACK:
                    ESP_LOGI(TAG, "power default %d", param->value.set.power_default.power);
                    break;
                case ESP_BLE_MESH_MODEL_OP_GEN_POWER_LEVEL_SET:
                case ESP_BLE_MESH_MODEL_OP_GEN_POWER_LEVEL_SET_UNACK:
                    ESP_LOGI(TAG, "power level %d, tid %d", param->value.set.power_level.power, param->value.set.power_level.tid);
                    break;
                case ESP_BLE_MESH_MODEL_OP_GEN_POWER_RANGE_SET:
                case ESP_BLE_MESH_MODEL_OP_GEN_POWER_RANGE_SET_UNACK:
                    ESP_LOGI(TAG, "power range min %d, max %d", param->value.set.power_range.range_min, param->value.set.power_range.range_max);
                    break;
                case ESP_BLE_MESH_MODEL_OP_GEN_LOC_GLOBAL_SET:
                case ESP_BLE_MESH_MODEL_OP_GEN_LOC_GLOBAL_SET_UNACK:
                    ESP_LOGI(TAG, "global location lat %d, long %d, alt %d",
                        param->value.set.location_global.global_latitude, param->value.set.location_global.global_longitude,
                        param->value.set.location_global.global_altitude);
                    break;
                case ESP_BLE_MESH_MODEL_OP_GEN_LOC_LOCAL_SET:
                case ESP_BLE_MESH_MODEL_OP_GEN_LOC_LOCAL_SET_UNACK:
                    ESP_LOGI(TAG, "local location north %d, east %d, alt %d, floor %d, uncertainty %d",
                        param->value.set.location_local.local_north, param->value.set.location_local.local_east,
                        param->value.set.location_local.local_altitude, param->value.set.location_local.floor_number,
                        param->value.set.location_local.uncertainty);
                    break;
                case ESP_BLE_MESH_MODEL_OP_GEN_ADMIN_PROPERTY_SET:
                case ESP_BLE_MESH_MODEL_OP_GEN_ADMIN_PROPERTY_SET_UNACK:
                    ESP_LOGI(TAG, "admin property id %d, access %d", param->value.set.admin_property.property_id, param->value.set.admin_property.user_access);
                    break;
                case ESP_BLE_MESH_MODEL_OP_GEN_MANUFACTURER_PROPERTY_SET:
                case ESP_BLE_MESH_MODEL_OP_GEN_MANUFACTURER_PROPERTY_SET_UNACK:
                    ESP_LOGI(TAG, "manufacturer property id %d, access %d", param->value.set.manu_property.property_id, param->value.set.manu_property.user_access);
                    break;
                case ESP_BLE_MESH_MODEL_OP_GEN_USER_PROPERTY_SET:
                case ESP_BLE_MESH_MODEL_OP_GEN_USER_PROPERTY_SET_UNACK:
                    ESP_LOGI(TAG, "user property id %d", param->value.set.user_property.property_id);
                    break;
                default:
                    break;
            }

            break;
        default:
            break;
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

static void mesh_config_server_cb(esp_ble_mesh_cfg_server_cb_event_t event,
                                  esp_ble_mesh_cfg_server_cb_param_t *param)
{
    if (event == ESP_BLE_MESH_CFG_SERVER_STATE_CHANGE_EVT) {
        switch (param->ctx.recv_op) {
        case ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD:
            ESP_LOGI(TAG, "ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD");
            ESP_LOGI(TAG, "net_idx 0x%04x, app_idx 0x%04x",
                param->value.state_change.appkey_add.net_idx,
                param->value.state_change.appkey_add.app_idx);
            ESP_LOG_BUFFER_HEX("AppKey", param->value.state_change.appkey_add.app_key, 16);
            break;
        case ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND:
            ESP_LOGI(TAG, "ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND");
            ESP_LOGI(TAG, "elem_addr 0x%04x, app_idx 0x%04x, cid 0x%04x, mod_id 0x%04x",
                param->value.state_change.mod_app_bind.element_addr,
                param->value.state_change.mod_app_bind.app_idx,
                param->value.state_change.mod_app_bind.company_id,
                param->value.state_change.mod_app_bind.model_id);
            break;
        case ESP_BLE_MESH_MODEL_OP_MODEL_SUB_ADD:
            ESP_LOGI(TAG, "ESP_BLE_MESH_MODEL_OP_MODEL_SUB_ADD");
            ESP_LOGI(TAG, "elem_addr 0x%04x, sub_addr 0x%04x, cid 0x%04x, mod_id 0x%04x",
                param->value.state_change.mod_sub_add.element_addr,
                param->value.state_change.mod_sub_add.sub_addr,
                param->value.state_change.mod_sub_add.company_id,
                param->value.state_change.mod_sub_add.model_id);
            break;
        default:
            break;
        }
    }
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
