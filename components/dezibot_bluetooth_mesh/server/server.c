#include "server.h"
#include "common/bluetooth.h"
#include "common/common.h"

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
        {
            esp_ble_mesh_server_model_send_msg(model, ctx,
               ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_STATUS, sizeof(srv->state.onoff), &srv->state.onoff);
            break;
        }
        case ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_SET:
        case ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_SET_UNACK:
        {
            srv->state.onoff = set->onoff;
            if (ctx->recv_op == ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_SET) {
                esp_ble_mesh_server_model_send_msg(model, ctx,
                    ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_STATUS, sizeof(srv->state.onoff), &srv->state.onoff);
            }
            esp_ble_mesh_model_publish(model, ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_STATUS,
                sizeof(srv->state.onoff), &srv->state.onoff, ROLE_NODE);
            break;
        }
        default:
            break;
    }
}

static void handle_gen_level_msg(esp_ble_mesh_model_t *model,
                                    esp_ble_mesh_msg_ctx_t *ctx,
                                    esp_ble_mesh_server_recv_gen_level_set_t *set_level,
                                    esp_ble_mesh_server_recv_gen_delta_set_t *set_delta,
                                    esp_ble_mesh_server_recv_gen_move_set_t *set_move)
{
    esp_ble_mesh_gen_level_srv_t *srv = model->user_data;

    switch (ctx->recv_op)
    {
        case ESP_BLE_MESH_MODEL_OP_GEN_LEVEL_GET:
        {
            esp_ble_mesh_server_model_send_msg(model, ctx,
               ESP_BLE_MESH_MODEL_OP_GEN_LEVEL_STATUS, sizeof(srv->state.level), (uint8_t *)&srv->state.level);
            break;
        }
        case ESP_BLE_MESH_MODEL_OP_GEN_LEVEL_SET:
        case ESP_BLE_MESH_MODEL_OP_GEN_LEVEL_SET_UNACK:
        {
            srv->state.target_level = set_level->level;
            if (ctx->recv_op == ESP_BLE_MESH_MODEL_OP_GEN_LEVEL_SET) {
                esp_ble_mesh_server_model_send_msg(model, ctx,
                    ESP_BLE_MESH_MODEL_OP_GEN_LEVEL_STATUS, sizeof(srv->state.level), (uint8_t *)&srv->state.level);
            }
            esp_ble_mesh_model_publish(model, ESP_BLE_MESH_MODEL_OP_GEN_LEVEL_STATUS,
                sizeof(srv->state.level), (uint8_t *)&srv->state.level, ROLE_NODE);
            break;
        }
        case ESP_BLE_MESH_MODEL_OP_GEN_DELTA_SET:
        case ESP_BLE_MESH_MODEL_OP_GEN_DELTA_SET_UNACK:
        {
            srv->state.target_level += set_delta->delta_level;
            if (ctx->recv_op == ESP_BLE_MESH_MODEL_OP_GEN_LEVEL_SET) {
                esp_ble_mesh_server_model_send_msg(model, ctx,
                    ESP_BLE_MESH_MODEL_OP_GEN_LEVEL_STATUS, sizeof(srv->state.level), (uint8_t *)&srv->state.level);
            }
            esp_ble_mesh_model_publish(model, ESP_BLE_MESH_MODEL_OP_GEN_LEVEL_STATUS,
                sizeof(srv->state.level), (uint8_t *)&srv->state.level, ROLE_NODE);
            break;
        }
        case ESP_BLE_MESH_MODEL_OP_GEN_MOVE_SET:
        case ESP_BLE_MESH_MODEL_OP_GEN_MOVE_SET_UNACK:
        {
            srv->state.target_level += set_move->delta_level;
            if (ctx->recv_op == ESP_BLE_MESH_MODEL_OP_GEN_LEVEL_SET) {
                esp_ble_mesh_server_model_send_msg(model, ctx,
                    ESP_BLE_MESH_MODEL_OP_GEN_LEVEL_STATUS, sizeof(srv->state.level), (uint8_t *)&srv->state.level);
            }
            esp_ble_mesh_model_publish(model, ESP_BLE_MESH_MODEL_OP_GEN_LEVEL_STATUS,
                sizeof(srv->state.level), (uint8_t *)&srv->state.level, ROLE_NODE);
            break;
        }
        default:
            break;
    }
}

static void handle_gen_def_trans_time_msg(esp_ble_mesh_model_t *model,
                                         esp_ble_mesh_msg_ctx_t *ctx,
                                         esp_ble_mesh_server_recv_gen_def_trans_time_set_t *set)
{
    esp_ble_mesh_gen_def_trans_time_srv_t *srv = model->user_data;

    switch (ctx->recv_op) {
        case ESP_BLE_MESH_MODEL_OP_GEN_DEF_TRANS_TIME_GET:
        {
            esp_ble_mesh_server_model_send_msg(model, ctx,
               ESP_BLE_MESH_MODEL_OP_GEN_DEF_TRANS_TIME_STATUS, sizeof(srv->state.trans_time), &srv->state.trans_time);
            break;
        }
        case ESP_BLE_MESH_MODEL_OP_GEN_DEF_TRANS_TIME_SET:
        case ESP_BLE_MESH_MODEL_OP_GEN_DEF_TRANS_TIME_SET_UNACK:
        {
            srv->state.trans_time = set->trans_time;
            if (ctx->recv_op == ESP_BLE_MESH_MODEL_OP_GEN_DEF_TRANS_TIME_SET) {
                esp_ble_mesh_server_model_send_msg(model, ctx,
                    ESP_BLE_MESH_MODEL_OP_GEN_DEF_TRANS_TIME_STATUS, sizeof(srv->state.trans_time), &srv->state.trans_time);
            }
            esp_ble_mesh_model_publish(model, ESP_BLE_MESH_MODEL_OP_GEN_DEF_TRANS_TIME_STATUS,
                sizeof(srv->state.trans_time), &srv->state.trans_time, ROLE_NODE);
            break;
        }
        default:
            break;
    }
}

static void handle_gen_onpowerup_msg(esp_ble_mesh_model_t *model,
                                         esp_ble_mesh_msg_ctx_t *ctx,
                                         esp_ble_mesh_server_recv_gen_onpowerup_set_t *set)
{
    esp_ble_mesh_gen_power_onoff_srv_t *srv = model->user_data;

    switch (ctx->recv_op) {
        case ESP_BLE_MESH_MODEL_OP_GEN_ONPOWERUP_GET:
        {
            esp_ble_mesh_server_model_send_msg(model, ctx,
               ESP_BLE_MESH_MODEL_OP_GEN_ONPOWERUP_STATUS, sizeof(srv->state->onpowerup), &srv->state->onpowerup);
            break;
        }
        case ESP_BLE_MESH_MODEL_OP_GEN_ONPOWERUP_SET:
        case ESP_BLE_MESH_MODEL_OP_GEN_ONPOWERUP_SET_UNACK:
        {
            srv->state->onpowerup = set->onpowerup;
            if (ctx->recv_op == ESP_BLE_MESH_MODEL_OP_GEN_ONPOWERUP_SET) {
                esp_ble_mesh_server_model_send_msg(model, ctx,
                    ESP_BLE_MESH_MODEL_OP_GEN_ONPOWERUP_STATUS, sizeof(srv->state->onpowerup), &srv->state->onpowerup);
            }
            esp_ble_mesh_model_publish(model, ESP_BLE_MESH_MODEL_OP_GEN_ONPOWERUP_STATUS,
                sizeof(srv->state->onpowerup), &srv->state->onpowerup, ROLE_NODE);
            break;
        }
        default:
            break;
    }
}

static void handle_gen_power_level_msg(esp_ble_mesh_model_t *model,
                                         esp_ble_mesh_msg_ctx_t *ctx,
                                         esp_ble_mesh_server_recv_gen_power_level_set_t *set_level,
                                         esp_ble_mesh_server_recv_gen_power_default_set_t *set_default,
                                         esp_ble_mesh_server_recv_gen_power_range_set_t *set_range)
{
    esp_ble_mesh_gen_power_level_srv_t *srv = model->user_data;

    switch (ctx->recv_op) {
        case ESP_BLE_MESH_MODEL_OP_GEN_POWER_LAST_GET:
        {
            esp_ble_mesh_server_model_send_msg(model, ctx,
               ESP_BLE_MESH_MODEL_OP_GEN_POWER_LAST_STATUS, sizeof(srv->state->power_last), (uint8_t *)&srv->state->power_last);
            break;
        }
        case ESP_BLE_MESH_MODEL_OP_GEN_POWER_LEVEL_GET:
        {
            esp_ble_mesh_server_model_send_msg(model, ctx,
               ESP_BLE_MESH_MODEL_OP_GEN_POWER_LEVEL_STATUS, sizeof(srv->state->power_actual), (uint8_t *)&srv->state->power_actual);
            break;
        }
        case ESP_BLE_MESH_MODEL_OP_GEN_POWER_LEVEL_SET:
        case ESP_BLE_MESH_MODEL_OP_GEN_POWER_LEVEL_SET_UNACK:
        {
            srv->state->target_power_actual = set_level->power;
            if (ctx->recv_op == ESP_BLE_MESH_MODEL_OP_GEN_POWER_LEVEL_SET) {
                esp_ble_mesh_server_model_send_msg(model, ctx,
                    ESP_BLE_MESH_MODEL_OP_GEN_POWER_LEVEL_STATUS, sizeof(srv->state->power_actual), (uint8_t *)&srv->state->power_actual);
            }
            esp_ble_mesh_model_publish(model, ESP_BLE_MESH_MODEL_OP_GEN_POWER_LEVEL_STATUS,
                sizeof(srv->state->power_actual), (uint8_t *)&srv->state->power_actual, ROLE_NODE);
            break;
        }
        case ESP_BLE_MESH_MODEL_OP_GEN_POWER_DEFAULT_GET:
        {
            esp_ble_mesh_server_model_send_msg(model, ctx,
               ESP_BLE_MESH_MODEL_OP_GEN_POWER_DEFAULT_STATUS, sizeof(srv->state->power_default), (uint8_t *)&srv->state->power_default);
            break;
        }
        case ESP_BLE_MESH_MODEL_OP_GEN_POWER_DEFAULT_SET:
        case ESP_BLE_MESH_MODEL_OP_GEN_POWER_DEFAULT_SET_UNACK:
        {
            srv->state->power_default = set_default->power;
            if (ctx->recv_op == ESP_BLE_MESH_MODEL_OP_GEN_POWER_DEFAULT_SET) {
                esp_ble_mesh_server_model_send_msg(model, ctx,
                    ESP_BLE_MESH_MODEL_OP_GEN_POWER_DEFAULT_STATUS, sizeof(srv->state->power_default), (uint8_t *)&srv->state->power_default);
            }
            esp_ble_mesh_model_publish(model, ESP_BLE_MESH_MODEL_OP_GEN_POWER_DEFAULT_STATUS,
                sizeof(srv->state->power_default), (uint8_t *)&srv->state->power_default, ROLE_NODE);
            break;
        }
        case ESP_BLE_MESH_MODEL_OP_GEN_POWER_RANGE_GET:
        {
            uint8_t data_power_range_get[5];
            struct net_buf_simple buf_power_range_get;
            net_buf_simple_init_with_data(&buf_power_range_get, data_power_range_get, sizeof(data_power_range_get));
            net_buf_simple_add_u8(&buf_power_range_get, srv->state->status_code);
            net_buf_simple_add_le16(&buf_power_range_get, srv->state->power_range_min);
            net_buf_simple_add_le16(&buf_power_range_get, srv->state->power_range_max);

            esp_ble_mesh_server_model_send_msg(model, ctx,
                ESP_BLE_MESH_MODEL_OP_GEN_POWER_RANGE_STATUS, buf_power_range_get.len, buf_power_range_get.data);
            break;
        }
        case ESP_BLE_MESH_MODEL_OP_GEN_POWER_RANGE_SET:
        case ESP_BLE_MESH_MODEL_OP_GEN_POWER_RANGE_SET_UNACK:
        {
            srv->state->power_range_min = set_range->range_min;
            srv->state->power_range_max = set_range->range_max;

            uint8_t data_power_range_set[5];
            struct net_buf_simple buf_power_range_set;
            net_buf_simple_init_with_data(&buf_power_range_set, data_power_range_set, sizeof(data_power_range_set));
            net_buf_simple_add_u8(&buf_power_range_set, srv->state->status_code);
            net_buf_simple_add_le16(&buf_power_range_set, srv->state->power_range_min);
            net_buf_simple_add_le16(&buf_power_range_set, srv->state->power_range_max);

            if (ctx->recv_op == ESP_BLE_MESH_MODEL_OP_GEN_POWER_DEFAULT_SET) {
                esp_ble_mesh_server_model_send_msg(model, ctx,
                    ESP_BLE_MESH_MODEL_OP_GEN_POWER_RANGE_STATUS, buf_power_range_set.len, buf_power_range_set.data);
            }
            esp_ble_mesh_model_publish(model, ESP_BLE_MESH_MODEL_OP_GEN_POWER_RANGE_STATUS,
                buf_power_range_set.len, buf_power_range_set.data, ROLE_NODE);
            break;
        }
        default:
            break;
    }
}

static void handle_gen_battery_msg(esp_ble_mesh_model_t *model,
                                         esp_ble_mesh_msg_ctx_t *ctx,
                                         esp_ble_mesh_server_recv_gen_onpowerup_set_t *set)
{
    esp_ble_mesh_gen_battery_srv_t*srv = model->user_data;

    switch (ctx->recv_op) {
    case ESP_BLE_MESH_MODEL_OP_GEN_BATTERY_GET:
    {
        uint8_t data_battery_get[8];
        struct net_buf_simple buf_battery_get;
        net_buf_simple_init_with_data(&buf_battery_get, data_battery_get, sizeof(data_battery_get));
        net_buf_simple_add_u8(&buf_battery_get, srv->state.battery_level);
        net_buf_simple_add_le24(&buf_battery_get, srv->state.time_to_discharge);
        net_buf_simple_add_le24(&buf_battery_get, srv->state.time_to_charge);
        net_buf_simple_add_u8(&buf_battery_get, srv->state.battery_flags);

        esp_ble_mesh_server_model_send_msg(model, ctx,
            ESP_BLE_MESH_MODEL_OP_GEN_BATTERY_STATUS, buf_battery_get.len, buf_battery_get.data);
        break;
    }
    default:
        break;
    }
}

static void handle_gen_location_msg(esp_ble_mesh_model_t *model,
                                         esp_ble_mesh_msg_ctx_t *ctx,
                                         esp_ble_mesh_server_recv_gen_loc_global_set_t *set_global,
                                         esp_ble_mesh_server_recv_gen_loc_local_set_t *set_local)
{
    esp_ble_mesh_gen_location_srv_t *srv = model->user_data;

    switch (ctx->recv_op) {
        case ESP_BLE_MESH_MODEL_OP_GEN_LOC_GLOBAL_GET:
        {
            uint8_t data_loc_global_get[10];
            struct net_buf_simple buf_loc_global_get;
            net_buf_simple_init_with_data(&buf_loc_global_get, data_loc_global_get, sizeof(data_loc_global_get));
            net_buf_simple_add_le32(&buf_loc_global_get, srv->state->global_latitude);
            net_buf_simple_add_le32(&buf_loc_global_get, srv->state->global_longitude);
            net_buf_simple_add_le16(&buf_loc_global_get, srv->state->global_altitude);

            esp_ble_mesh_server_model_send_msg(model, ctx,
                ESP_BLE_MESH_MODEL_OP_GEN_LOC_GLOBAL_STATUS, buf_loc_global_get.len, buf_loc_global_get.data);
            break;
        }
        case ESP_BLE_MESH_MODEL_OP_GEN_LOC_GLOBAL_SET:
        case ESP_BLE_MESH_MODEL_OP_GEN_LOC_GLOBAL_SET_UNACK:
        {
            srv->state->global_latitude = set_global->global_latitude;
            srv->state->global_longitude = set_global->global_longitude;
            srv->state->global_altitude = set_global->global_altitude;

            uint8_t data_loc_global_set[10];
            struct net_buf_simple buf_loc_global_set;
            net_buf_simple_init_with_data(&buf_loc_global_set, data_loc_global_set, sizeof(data_loc_global_set));
            net_buf_simple_add_le32(&buf_loc_global_set, srv->state->global_latitude);
            net_buf_simple_add_le32(&buf_loc_global_set, srv->state->global_longitude);
            net_buf_simple_add_le16(&buf_loc_global_set, srv->state->global_altitude);

            if (ctx->recv_op == ESP_BLE_MESH_MODEL_OP_GEN_LOC_GLOBAL_SET) {
                esp_ble_mesh_server_model_send_msg(model, ctx,
                    ESP_BLE_MESH_MODEL_OP_GEN_LOC_GLOBAL_STATUS, buf_loc_global_set.len, buf_loc_global_set.data);
            }
            esp_ble_mesh_model_publish(model, ESP_BLE_MESH_MODEL_OP_GEN_LOC_GLOBAL_STATUS,
                buf_loc_global_set.len, buf_loc_global_set.data, ROLE_NODE);
            break;
        }
        case ESP_BLE_MESH_MODEL_OP_GEN_LOC_LOCAL_GET:
        {
            uint8_t data_loc_local_get[9];
            struct net_buf_simple buf_loc_local_get;
            net_buf_simple_init_with_data(&buf_loc_local_get, data_loc_local_get, sizeof(data_loc_local_get));
            net_buf_simple_add_le16(&buf_loc_local_get, srv->state->local_north);
            net_buf_simple_add_le16(&buf_loc_local_get, srv->state->local_east);
            net_buf_simple_add_le16(&buf_loc_local_get, srv->state->local_altitude);
            net_buf_simple_add_u8(&buf_loc_local_get, srv->state->floor_number);
            net_buf_simple_add_le16(&buf_loc_local_get, srv->state->uncertainty);

            esp_ble_mesh_server_model_send_msg(model, ctx,
                ESP_BLE_MESH_MODEL_OP_GEN_LOC_LOCAL_STATUS, buf_loc_local_get.len, buf_loc_local_get.data);
            break;
        }
        case ESP_BLE_MESH_MODEL_OP_GEN_LOC_LOCAL_SET:
        case ESP_BLE_MESH_MODEL_OP_GEN_LOC_LOCAL_SET_UNACK:
        {
            srv->state->local_north = set_local->local_north;
            srv->state->local_east = set_local->local_east;
            srv->state->local_altitude = set_local->local_altitude;
            srv->state->floor_number = set_local->floor_number;
            srv->state->uncertainty = set_local->uncertainty;

            uint8_t data_loc_local_set[9];
            struct net_buf_simple buf_loc_local_set;
            net_buf_simple_init_with_data(&buf_loc_local_set, data_loc_local_set, sizeof(data_loc_local_set));
            net_buf_simple_add_le16(&buf_loc_local_set, srv->state->local_north);
            net_buf_simple_add_le16(&buf_loc_local_set, srv->state->local_east);
            net_buf_simple_add_le16(&buf_loc_local_set, srv->state->local_altitude);
            net_buf_simple_add_u8(&buf_loc_local_set, srv->state->floor_number);
            net_buf_simple_add_le16(&buf_loc_local_set, srv->state->uncertainty);

            if (ctx->recv_op == ESP_BLE_MESH_MODEL_OP_GEN_LOC_LOCAL_SET) {
                esp_ble_mesh_server_model_send_msg(model, ctx,
                    ESP_BLE_MESH_MODEL_OP_GEN_LOC_LOCAL_STATUS, buf_loc_local_set.len, buf_loc_local_set.data);
            }
            esp_ble_mesh_model_publish(model, ESP_BLE_MESH_MODEL_OP_GEN_LOC_LOCAL_STATUS,
                buf_loc_local_set.len, buf_loc_local_set.data, ROLE_NODE);
            break;
        }
        default:
            break;
    }
}

static void handle_gen_client_prop_msg(esp_ble_mesh_model_t *model,
                                         esp_ble_mesh_msg_ctx_t *ctx)
{
    esp_ble_mesh_gen_client_prop_srv_t *srv = model->user_data;

    switch (ctx->recv_op)
    {
        case ESP_BLE_MESH_MODEL_OP_GEN_CLIENT_PROPERTIES_GET:
        {
            uint8_t data_client_props_get[2*srv->id_count];
            struct net_buf_simple buf_client_props_get;
            net_buf_simple_init_with_data(&buf_client_props_get, data_client_props_get, sizeof(data_client_props_get));
            for (uint8_t i = 0; i < srv->id_count; i++)
            {
                net_buf_simple_add_le16(&buf_client_props_get, srv->property_ids[i]);
            }

            esp_ble_mesh_server_model_send_msg(model, ctx,
                ESP_BLE_MESH_MODEL_OP_GEN_CLIENT_PROPERTIES_STATUS, buf_client_props_get.len, buf_client_props_get.data);
            break;
        }
        default:
            break;
    }
}

static void handle_gen_admin_prop_msg(esp_ble_mesh_model_t *model,
                                         esp_ble_mesh_msg_ctx_t *ctx,
                                         esp_ble_mesh_server_recv_gen_admin_property_get_t *get,
                                         esp_ble_mesh_server_recv_gen_admin_property_set_t *set)
{
    esp_ble_mesh_gen_admin_prop_srv_t *srv = model->user_data;

    switch (ctx->recv_op) {
        case ESP_BLE_MESH_MODEL_OP_GEN_ADMIN_PROPERTIES_GET:
        {
            uint8_t data_admin_props_get[2*srv->property_count];
            struct net_buf_simple buf_admin_props_get;
            net_buf_simple_init_with_data(&buf_admin_props_get, data_admin_props_get, sizeof(data_admin_props_get));
            for (uint8_t i = 0; i < srv->property_count; i++)
            {
                net_buf_simple_add_le16(&buf_admin_props_get, srv->properties[i].id);
            }

            esp_ble_mesh_server_model_send_msg(model, ctx,
                ESP_BLE_MESH_MODEL_OP_GEN_ADMIN_PROPERTIES_STATUS, buf_admin_props_get.len, buf_admin_props_get.data);
            break;
        }
        case ESP_BLE_MESH_MODEL_OP_GEN_ADMIN_PROPERTY_GET:
        {
            uint16_t prop_id;
            for (uint8_t i = 0; i < srv->property_count; i++)
            {
                if (srv->properties[i].id == get->property_id) {
                    prop_id = srv->properties[i].id;
                    break;
                }
            }

            esp_ble_mesh_server_model_send_msg(model, ctx,
                ESP_BLE_MESH_MODEL_OP_GEN_ADMIN_PROPERTY_STATUS, sizeof(prop_id), (uint8_t *)&prop_id);
            break;
        }
        case ESP_BLE_MESH_MODEL_OP_GEN_ADMIN_PROPERTY_SET:
        case ESP_BLE_MESH_MODEL_OP_GEN_ADMIN_PROPERTY_SET_UNACK:
        {
            bool prop_already_exists = false;
            uint8_t prop_index = 0;
            for (uint8_t i = 0; i < srv->property_count; i++)
            {
                if (srv->properties[i].id == set->property_id) {
                    prop_already_exists = true;
                    prop_index = i;
                    break;
                }
            }

            if (prop_already_exists)
            {
                srv->properties[prop_index].admin_access = set->user_access;
                srv->properties[prop_index].val = set->property_value;
            } else {
                srv->properties[srv->property_count].id = set->property_id;
                srv->properties[srv->property_count].admin_access = set->user_access;
                srv->properties[srv->property_count].val = set->property_value;
                srv->property_count++;
            }

            if (ctx->recv_op == ESP_BLE_MESH_MODEL_OP_GEN_ADMIN_PROPERTY_SET) {
                esp_ble_mesh_server_model_send_msg(model, ctx,
                    ESP_BLE_MESH_MODEL_OP_GEN_ADMIN_PROPERTY_STATUS, sizeof(set->property_id), (uint8_t *)&set->property_id);
            }
            esp_ble_mesh_model_publish(model, ESP_BLE_MESH_MODEL_OP_GEN_ADMIN_PROPERTY_STATUS,
                sizeof(set->property_id), (uint8_t *)&set->property_id, ROLE_NODE);
            break;
        }
        default:
            break;
    }
}

static void handle_gen_manu_prop_msg(esp_ble_mesh_model_t *model,
                                         esp_ble_mesh_msg_ctx_t *ctx,
                                         esp_ble_mesh_server_recv_gen_manufacturer_property_get_t *get,
                                         esp_ble_mesh_server_recv_gen_manufacturer_property_set_t *set)
{
    esp_ble_mesh_gen_manu_prop_srv_t *srv = model->user_data;

    switch (ctx->recv_op) {
        case ESP_BLE_MESH_MODEL_OP_GEN_MANUFACTURER_PROPERTIES_GET:
        {
            uint8_t data_manu_props_get[2*srv->property_count];
            struct net_buf_simple buf_manu_props_get;
            net_buf_simple_init_with_data(&buf_manu_props_get, data_manu_props_get, sizeof(data_manu_props_get));
            for (uint8_t i = 0; i < srv->property_count; i++)
            {
                net_buf_simple_add_le16(&buf_manu_props_get, srv->properties[i].id);
            }

            esp_ble_mesh_server_model_send_msg(model, ctx,
                ESP_BLE_MESH_MODEL_OP_GEN_MANUFACTURER_PROPERTIES_STATUS, buf_manu_props_get.len, buf_manu_props_get.data);
            break;
        }
        case ESP_BLE_MESH_MODEL_OP_GEN_MANUFACTURER_PROPERTY_GET:
        {
            uint16_t prop_id;
            for (uint8_t i = 0; i < srv->property_count; i++)
            {
                if (srv->properties[i].id == get->property_id) {
                    prop_id = srv->properties[i].id;
                    break;
                }
            }

            esp_ble_mesh_server_model_send_msg(model, ctx,
                ESP_BLE_MESH_MODEL_OP_GEN_MANUFACTURER_PROPERTY_STATUS, sizeof(prop_id), (uint8_t *)&prop_id);
            break;
        }
        case ESP_BLE_MESH_MODEL_OP_GEN_MANUFACTURER_PROPERTY_SET:
        case ESP_BLE_MESH_MODEL_OP_GEN_MANUFACTURER_PROPERTY_SET_UNACK:
        {
            bool prop_already_exists = false;
            uint8_t prop_index = 0;
            for (uint8_t i = 0; i < srv->property_count; i++)
            {
                if (srv->properties[i].id == set->property_id) {
                    prop_already_exists = true;
                    prop_index = i;
                    break;
                }
            }

            if (prop_already_exists)
            {
                srv->properties[prop_index].manu_access = set->user_access;
            } else {
                srv->properties[srv->property_count].id = set->property_id;
                srv->properties[srv->property_count].manu_access = set->user_access;
                srv->property_count++;
            }

            if (ctx->recv_op == ESP_BLE_MESH_MODEL_OP_GEN_MANUFACTURER_PROPERTY_SET) {
                esp_ble_mesh_server_model_send_msg(model, ctx,
                    ESP_BLE_MESH_MODEL_OP_GEN_MANUFACTURER_PROPERTY_STATUS, sizeof(set->property_id), (uint8_t *)&set->property_id);
            }
            esp_ble_mesh_model_publish(model, ESP_BLE_MESH_MODEL_OP_GEN_MANUFACTURER_PROPERTY_STATUS,
                sizeof(set->property_id), (uint8_t *)&set->property_id, ROLE_NODE);
            break;
        }
        default:
            break;
    }
}

static void handle_gen_user_prop_msg(esp_ble_mesh_model_t *model,
                                         esp_ble_mesh_msg_ctx_t *ctx,
                                         esp_ble_mesh_server_recv_gen_user_property_get_t *get,
                                         esp_ble_mesh_server_recv_gen_user_property_set_t *set)
{
    esp_ble_mesh_gen_user_prop_srv_t *srv = model->user_data;

    switch (ctx->recv_op) {
        case ESP_BLE_MESH_MODEL_OP_GEN_USER_PROPERTIES_GET:
        {
            uint8_t data_user_props_get[2*srv->property_count];
            struct net_buf_simple buf_user_props_get;
            net_buf_simple_init_with_data(&buf_user_props_get, data_user_props_get, sizeof(data_user_props_get));
            for (uint8_t i = 0; i < srv->property_count; i++)
            {
                net_buf_simple_add_le16(&buf_user_props_get, srv->properties[i].id);
            }

            esp_ble_mesh_server_model_send_msg(model, ctx,
                ESP_BLE_MESH_MODEL_OP_GEN_USER_PROPERTIES_STATUS, buf_user_props_get.len, buf_user_props_get.data);
            break;
        }
        case ESP_BLE_MESH_MODEL_OP_GEN_USER_PROPERTY_GET:
        {
            uint16_t prop_id;
            for (uint8_t i = 0; i < srv->property_count; i++)
            {
                if (srv->properties[i].id == get->property_id) {
                    prop_id = srv->properties[i].id;
                    break;
                }
            }

            esp_ble_mesh_server_model_send_msg(model, ctx,
                ESP_BLE_MESH_MODEL_OP_GEN_USER_PROPERTY_STATUS, sizeof(prop_id), (uint8_t *)&prop_id);
            break;
        }
        case ESP_BLE_MESH_MODEL_OP_GEN_USER_PROPERTY_SET:
        case ESP_BLE_MESH_MODEL_OP_GEN_USER_PROPERTY_SET_UNACK:
        {
            bool prop_already_exists = false;
            uint8_t prop_index = 0;
            for (uint8_t i = 0; i < srv->property_count; i++)
            {
                if (srv->properties[i].id == set->property_id) {
                    prop_already_exists = true;
                    prop_index = i;
                    break;
                }
            }

            if (prop_already_exists)
            {
                srv->properties[prop_index].val = set->property_value;
            } else {
                srv->properties[srv->property_count].id = set->property_id;
                srv->properties[srv->property_count].val = set->property_value;
                srv->property_count++;
            }

            if (ctx->recv_op == ESP_BLE_MESH_MODEL_OP_GEN_USER_PROPERTY_SET) {
                esp_ble_mesh_server_model_send_msg(model, ctx,
                    ESP_BLE_MESH_MODEL_OP_GEN_USER_PROPERTY_STATUS, sizeof(set->property_id), (uint8_t *)&set->property_id);
            }
            esp_ble_mesh_model_publish(model, ESP_BLE_MESH_MODEL_OP_GEN_USER_PROPERTY_STATUS,
                sizeof(set->property_id), (uint8_t *)&set->property_id, ROLE_NODE);
            break;
        }
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
                {
                    uint8_t onoff_state = param->value.state_change.onoff_set.onoff;
                    ESP_LOGI(TAG, "OnOff state changed to %d", onoff_state);
                    break;
                }
                case ESP_BLE_MESH_MODEL_OP_GEN_LEVEL_SET:
                case ESP_BLE_MESH_MODEL_OP_GEN_LEVEL_SET_UNACK:
                {
                    int16_t level = param->value.state_change.level_set.level;
                    ESP_LOGI(TAG, "Level state changed to %d", level);
                    break;
                }
                case ESP_BLE_MESH_MODEL_OP_GEN_DELTA_SET:
                case ESP_BLE_MESH_MODEL_OP_GEN_DELTA_SET_UNACK:
                {
                    int16_t delta = param->value.state_change.delta_set.level;
                    ESP_LOGI(TAG, "Delta state changed by %d", delta);
                    break;
                }
                case ESP_BLE_MESH_MODEL_OP_GEN_MOVE_SET:
                case ESP_BLE_MESH_MODEL_OP_GEN_MOVE_SET_UNACK:
                {
                    int16_t move_level = param->value.state_change.move_set.level;
                    ESP_LOGI(TAG, "Move level changed to %d", move_level);
                    break;
                }
                case ESP_BLE_MESH_MODEL_OP_GEN_DEF_TRANS_TIME_SET:
                case ESP_BLE_MESH_MODEL_OP_GEN_DEF_TRANS_TIME_SET_UNACK:
                {
                    uint8_t trans_time = param->value.state_change.def_trans_time_set.trans_time;
                    ESP_LOGI(TAG, "Default Transition Time changed to %d", trans_time);
                    break;
                }
                case ESP_BLE_MESH_MODEL_OP_GEN_ONPOWERUP_SET:
                case ESP_BLE_MESH_MODEL_OP_GEN_ONPOWERUP_SET_UNACK:
                {
                    uint8_t onpowerup = param->value.state_change.onpowerup_set.onpowerup;
                    ESP_LOGI(TAG, "On Power Up state changed to %d", onpowerup);
                    break;
                }
                case ESP_BLE_MESH_MODEL_OP_GEN_POWER_DEFAULT_SET:
                case ESP_BLE_MESH_MODEL_OP_GEN_POWER_DEFAULT_SET_UNACK:
                {
                    uint16_t power_default = param->value.state_change.power_default_set.power;
                    ESP_LOGI(TAG, "Power Default changed to %d", power_default);
                    break;
                }
                case ESP_BLE_MESH_MODEL_OP_GEN_POWER_LEVEL_SET:
                case ESP_BLE_MESH_MODEL_OP_GEN_POWER_LEVEL_SET_UNACK:
                {
                    uint16_t power_level = param->value.state_change.power_level_set.power;
                    ESP_LOGI(TAG, "Power Level changed to %d", power_level);
                    break;
                }
                case ESP_BLE_MESH_MODEL_OP_GEN_POWER_RANGE_SET:
                case ESP_BLE_MESH_MODEL_OP_GEN_POWER_RANGE_SET_UNACK:
                {
                    uint16_t power_range_min = param->value.state_change.power_range_set.range_min;
                    uint16_t power_range_max = param->value.state_change.power_range_set.range_max;
                    ESP_LOGI(TAG, "Power Range changed to min: %d, max: %d", power_range_min, power_range_max);
                    break;
                }
                case ESP_BLE_MESH_MODEL_OP_GEN_LOC_GLOBAL_SET:
                case ESP_BLE_MESH_MODEL_OP_GEN_LOC_GLOBAL_SET_UNACK:
                {
                    int32_t latitude = param->value.state_change.loc_global_set.latitude;
                    int32_t longitude = param->value.state_change.loc_global_set.longitude;
                    int32_t altitude = param->value.state_change.loc_global_set.altitude;
                    ESP_LOGI(TAG, "Global Location changed to lat: %d, long: %d, alt: %d", latitude, longitude, altitude);
                    break;
                }
                case ESP_BLE_MESH_MODEL_OP_GEN_LOC_LOCAL_SET:
                case ESP_BLE_MESH_MODEL_OP_GEN_LOC_LOCAL_SET_UNACK:
                {
                    uint16_t north = param->value.state_change.loc_local_set.north;
                    uint16_t east = param->value.state_change.loc_local_set.east;
                    int16_t altitude_local = param->value.state_change.loc_local_set.altitude;
                    uint8_t floor_number = param->value.state_change.loc_local_set.floor_number;
                    uint16_t uncertainty = param->value.state_change.loc_local_set.uncertainty;
                    ESP_LOGI(TAG, "Local Location changed to north: %d, east: %d, alt: %d, floor: %d, uncertainty: %d",
                        north, east, altitude_local, floor_number, uncertainty);
                    break;
                }
                case ESP_BLE_MESH_MODEL_OP_GEN_ADMIN_PROPERTY_SET:
                case ESP_BLE_MESH_MODEL_OP_GEN_ADMIN_PROPERTY_SET_UNACK:
                {
                    uint16_t admin_prop_id = param->value.state_change.admin_property_set.id;
                    uint8_t admin_prop_access = param->value.state_change.admin_property_set.access;
                    ESP_LOGI(TAG, "Admin Property changed - ID: %d, Access: %d", admin_prop_id, admin_prop_access);
                    break;
                }
                case ESP_BLE_MESH_MODEL_OP_GEN_MANUFACTURER_PROPERTY_SET:
                case ESP_BLE_MESH_MODEL_OP_GEN_MANUFACTURER_PROPERTY_SET_UNACK:
                {
                    uint16_t manu_prop_id = param->value.state_change.manu_property_set.id;
                    uint8_t manu_prop_access = param->value.state_change.manu_property_set.access;
                    ESP_LOGI(TAG, "Manufacturer Property changed - ID: %d, Access: %d", manu_prop_id, manu_prop_access);
                    break;
                }
                case ESP_BLE_MESH_MODEL_OP_GEN_USER_PROPERTY_SET:
                case ESP_BLE_MESH_MODEL_OP_GEN_USER_PROPERTY_SET_UNACK:
                {
                    uint16_t user_prop_id = param->value.state_change.user_property_set.id;
                    ESP_LOGI(TAG, "User Property changed - ID: %d", user_prop_id);
                    break;
                }
                default:
                    break;
            }

            break;
        case ESP_BLE_MESH_GENERIC_SERVER_RECV_GET_MSG_EVT:
            ESP_LOGI(TAG, "ESP_BLE_MESH_GENERIC_SERVER_RECV_GET_MSG_EVT");

            switch (param->ctx.recv_op)
            {
                case ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_GET:
                {
                    const esp_ble_mesh_gen_onoff_srv_t *srv = param->model->user_data;
                    ESP_LOGI(TAG, "onoff %d", srv->state.onoff);
                    handle_gen_onoff_msg(param->model, &param->ctx, NULL);
                    break;
                }
                case ESP_BLE_MESH_MODEL_OP_GEN_LEVEL_GET:
                {
                    const esp_ble_mesh_gen_level_srv_t *level_srv = param->model->user_data;
                    ESP_LOGI(TAG, "level %d", level_srv->state.level);
                    handle_gen_level_msg(param->model, &param->ctx, NULL, NULL, NULL);
                    break;
                }
                case ESP_BLE_MESH_MODEL_OP_GEN_DEF_TRANS_TIME_GET:
                {
                    const esp_ble_mesh_gen_def_trans_time_srv_t *def_trans_time = param->model->user_data;
                    ESP_LOGI(TAG, "default transition time %d", def_trans_time->state.trans_time);
                    handle_gen_def_trans_time_msg(param->model, &param->ctx, NULL);
                    break;
                }
                case ESP_BLE_MESH_MODEL_OP_GEN_ONPOWERUP_GET:
                {
                    const esp_ble_mesh_gen_power_onoff_srv_t *power_onoff_srv = param->model->user_data;
                    ESP_LOGI(TAG, "on power up %d", power_onoff_srv->state->onpowerup);
                    handle_gen_onpowerup_msg(param->model, &param->ctx, NULL);
                    break;
                }
                case ESP_BLE_MESH_MODEL_OP_GEN_POWER_DEFAULT_GET:
                {
                    const esp_ble_mesh_gen_power_level_srv_t *power_default_srv = param->model->user_data;
                    ESP_LOGI(TAG, "power default %d", power_default_srv->state->power_default);
                    handle_gen_power_level_msg(param->model, &param->ctx, NULL, NULL, NULL);
                    break;
                }
                case ESP_BLE_MESH_MODEL_OP_GEN_POWER_LEVEL_GET:
                {
                    const esp_ble_mesh_gen_power_level_srv_t *power_level_srv = param->model->user_data;
                    ESP_LOGI(TAG, "power level %d", power_level_srv->state->power_actual);
                    handle_gen_power_level_msg(param->model, &param->ctx, NULL, NULL, NULL);
                    break;
                }
                case ESP_BLE_MESH_MODEL_OP_GEN_POWER_LAST_GET:
                {
                    const esp_ble_mesh_gen_power_level_srv_t *power_last_srv = param->model->user_data;
                    ESP_LOGI(TAG, "power last %d", power_last_srv->state->power_last);
                    handle_gen_power_level_msg(param->model, &param->ctx, NULL, NULL, NULL);
                    break;
                }
                case ESP_BLE_MESH_MODEL_OP_GEN_POWER_RANGE_GET:
                {
                    const esp_ble_mesh_gen_power_level_srv_t *power_range_srv = param->model->user_data;
                    ESP_LOGI(TAG, "power range min %d, max %d", power_range_srv->state->power_range_min, power_range_srv->state->power_range_max);
                    handle_gen_power_level_msg(param->model, &param->ctx, NULL, NULL, NULL);
                    break;
                }
                case ESP_BLE_MESH_MODEL_OP_GEN_BATTERY_GET:
                {
                    const esp_ble_mesh_gen_battery_srv_t *battery_srv = param->model->user_data;
                    ESP_LOGI(TAG, "battery level %d, time to discharge %d, time to charge %d, flags %d",
                        battery_srv->state.battery_level, battery_srv->state.time_to_discharge,
                        battery_srv->state.time_to_charge, battery_srv->state.battery_flags);
                    handle_gen_battery_msg(param->model, &param->ctx, NULL);
                    break;
                }
                case ESP_BLE_MESH_MODEL_OP_GEN_LOC_GLOBAL_GET:
                {
                    const esp_ble_mesh_gen_location_srv_t *loc_global_srv = param->model->user_data;
                    ESP_LOGI(TAG, "global location lat %d, long %d, alt %d",
                        loc_global_srv->state->global_latitude, loc_global_srv->state->global_longitude, loc_global_srv->state->global_altitude);
                    handle_gen_location_msg(param->model, &param->ctx, NULL, NULL);
                    break;
                }
                case ESP_BLE_MESH_MODEL_OP_GEN_LOC_LOCAL_GET:
                {
                    const esp_ble_mesh_gen_location_srv_t *loc_local_srv = param->model->user_data;
                    ESP_LOGI(TAG, "local location north %d, east %d, alt %d, floor %d, uncertainty %d",
                        loc_local_srv->state->local_north, loc_local_srv->state->local_east, loc_local_srv->state->local_altitude,
                        loc_local_srv->state->floor_number, loc_local_srv->state->uncertainty);
                    handle_gen_location_msg(param->model, &param->ctx, NULL, NULL);
                    break;
                }
                case ESP_BLE_MESH_MODEL_OP_GEN_CLIENT_PROPERTIES_GET:
                {
                    const esp_ble_mesh_gen_client_prop_srv_t *client_prop_srv = param->model->user_data;
                    ESP_LOGI(TAG, "client properties count %d", client_prop_srv->id_count);
                    handle_gen_client_prop_msg(param->model, &param->ctx);
                    break;
                }
                case ESP_BLE_MESH_MODEL_OP_GEN_ADMIN_PROPERTIES_GET:
                {
                    const esp_ble_mesh_gen_admin_prop_srv_t *admin_srv = param->model->user_data;
                    ESP_LOGI(TAG, "admin properties count %d", admin_srv->property_count);
                    handle_gen_admin_prop_msg(param->model, &param->ctx, NULL, NULL);
                    break;
                }
                case ESP_BLE_MESH_MODEL_OP_GEN_ADMIN_PROPERTY_GET:
                {
                    const esp_ble_mesh_gen_admin_prop_srv_t *admin_prop_srv = param->model->user_data;
                    ESP_LOGI(TAG, "admin property id %d", admin_prop_srv->properties->id);
                    handle_gen_admin_prop_msg(param->model, &param->ctx, &param->value.get.admin_property, NULL);
                    break;
                }
                case ESP_BLE_MESH_MODEL_OP_GEN_MANUFACTURER_PROPERTIES_GET:
                {
                    const esp_ble_mesh_gen_manu_prop_srv_t *manu_srv = param->model->user_data;
                    ESP_LOGI(TAG, "manufacturer properties count %d", manu_srv->property_count);
                    handle_gen_manu_prop_msg(param->model, &param->ctx, NULL, NULL);
                    break;
                }
                case ESP_BLE_MESH_MODEL_OP_GEN_MANUFACTURER_PROPERTY_GET:
                {
                    const esp_ble_mesh_gen_manu_prop_srv_t *manu_prop_srv = param->model->user_data;
                    ESP_LOGI(TAG, "manufacturer property id %d", manu_prop_srv->properties->id);
                    handle_gen_manu_prop_msg(param->model, &param->ctx, &param->value.get.manu_property, NULL);
                    break;
                }
                case ESP_BLE_MESH_MODEL_OP_GEN_USER_PROPERTIES_GET:
                {
                    const esp_ble_mesh_gen_user_prop_srv_t *user_srv = param->model->user_data;
                    ESP_LOGI(TAG, "user properties count %d", user_srv->property_count);
                    handle_gen_user_prop_msg(param->model, &param->ctx, NULL, NULL);
                    break;
                }
                case ESP_BLE_MESH_MODEL_OP_GEN_USER_PROPERTY_GET:
                {
                    const esp_ble_mesh_gen_user_prop_srv_t *user_prop_srv = param->model->user_data;
                    ESP_LOGI(TAG, "user property id %d", user_prop_srv->properties->id);
                    handle_gen_user_prop_msg(param->model, &param->ctx, &param->value.get.user_property, NULL);
                    break;
                }
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
                    handle_gen_level_msg(param->model, &param->ctx, &param->value.set.level, NULL, NULL);
                    break;
                case ESP_BLE_MESH_MODEL_OP_GEN_DELTA_SET:
                case ESP_BLE_MESH_MODEL_OP_GEN_DELTA_SET_UNACK:
                    ESP_LOGI(TAG, "delta %d, tid %d", param->value.set.delta.delta_level, param->value.set.delta.tid);
                    handle_gen_level_msg(param->model, &param->ctx, NULL, &param->value.set.delta, NULL);
                    break;
                case ESP_BLE_MESH_MODEL_OP_GEN_MOVE_SET:
                case ESP_BLE_MESH_MODEL_OP_GEN_MOVE_SET_UNACK:
                    ESP_LOGI(TAG, "move level %d, tid %d", param->value.set.move.delta_level, param->value.set.move.tid);
                    handle_gen_level_msg(param->model, &param->ctx, NULL, NULL, &param->value.set.move);
                    break;
                case ESP_BLE_MESH_MODEL_OP_GEN_DEF_TRANS_TIME_SET:
                case ESP_BLE_MESH_MODEL_OP_GEN_DEF_TRANS_TIME_SET_UNACK:
                    ESP_LOGI(TAG, "default transition time %d", param->value.set.def_trans_time.trans_time);
                    handle_gen_def_trans_time_msg(param->model, &param->ctx, &param->value.set.def_trans_time);
                    break;
                case ESP_BLE_MESH_MODEL_OP_GEN_ONPOWERUP_SET:
                case ESP_BLE_MESH_MODEL_OP_GEN_ONPOWERUP_SET_UNACK:
                    ESP_LOGI(TAG, "on power up %d", param->value.set.onpowerup.onpowerup);
                    handle_gen_onpowerup_msg(param->model, &param->ctx, &param->value.set.onpowerup);
                    break;
                case ESP_BLE_MESH_MODEL_OP_GEN_POWER_DEFAULT_SET:
                case ESP_BLE_MESH_MODEL_OP_GEN_POWER_DEFAULT_SET_UNACK:
                    ESP_LOGI(TAG, "power default %d", param->value.set.power_default.power);
                    handle_gen_power_level_msg(param->model, &param->ctx, NULL, &param->value.set.power_default, NULL);
                    break;
                case ESP_BLE_MESH_MODEL_OP_GEN_POWER_LEVEL_SET:
                case ESP_BLE_MESH_MODEL_OP_GEN_POWER_LEVEL_SET_UNACK:
                    ESP_LOGI(TAG, "power level %d, tid %d", param->value.set.power_level.power, param->value.set.power_level.tid);
                    handle_gen_power_level_msg(param->model, &param->ctx, &param->value.set.power_level, NULL, NULL);
                    break;
                case ESP_BLE_MESH_MODEL_OP_GEN_POWER_RANGE_SET:
                case ESP_BLE_MESH_MODEL_OP_GEN_POWER_RANGE_SET_UNACK:
                    ESP_LOGI(TAG, "power range min %d, max %d", param->value.set.power_range.range_min, param->value.set.power_range.range_max);
                    handle_gen_power_level_msg(param->model, &param->ctx, NULL, NULL, &param->value.set.power_range);
                    break;
                case ESP_BLE_MESH_MODEL_OP_GEN_LOC_GLOBAL_SET:
                case ESP_BLE_MESH_MODEL_OP_GEN_LOC_GLOBAL_SET_UNACK:
                    ESP_LOGI(TAG, "global location lat %d, long %d, alt %d",
                        param->value.set.location_global.global_latitude, param->value.set.location_global.global_longitude,
                        param->value.set.location_global.global_altitude);
                    handle_gen_location_msg(param->model, &param->ctx, &param->value.set.location_global, NULL);
                    break;
                case ESP_BLE_MESH_MODEL_OP_GEN_LOC_LOCAL_SET:
                case ESP_BLE_MESH_MODEL_OP_GEN_LOC_LOCAL_SET_UNACK:
                    ESP_LOGI(TAG, "local location north %d, east %d, alt %d, floor %d, uncertainty %d",
                        param->value.set.location_local.local_north, param->value.set.location_local.local_east,
                        param->value.set.location_local.local_altitude, param->value.set.location_local.floor_number,
                        param->value.set.location_local.uncertainty);
                    handle_gen_location_msg(param->model, &param->ctx, NULL, &param->value.set.location_local);
                    break;
                case ESP_BLE_MESH_MODEL_OP_GEN_ADMIN_PROPERTY_SET:
                case ESP_BLE_MESH_MODEL_OP_GEN_ADMIN_PROPERTY_SET_UNACK:
                    ESP_LOGI(TAG, "admin property id %d, access %d", param->value.set.admin_property.property_id, param->value.set.admin_property.user_access);
                    handle_gen_admin_prop_msg(param->model, &param->ctx, NULL, &param->value.set.admin_property);
                    break;
                case ESP_BLE_MESH_MODEL_OP_GEN_MANUFACTURER_PROPERTY_SET:
                case ESP_BLE_MESH_MODEL_OP_GEN_MANUFACTURER_PROPERTY_SET_UNACK:
                    ESP_LOGI(TAG, "manufacturer property id %d, access %d", param->value.set.manu_property.property_id, param->value.set.manu_property.user_access);
                    handle_gen_manu_prop_msg(param->model, &param->ctx, NULL, &param->value.set.manu_property);
                    break;
                case ESP_BLE_MESH_MODEL_OP_GEN_USER_PROPERTY_SET:
                case ESP_BLE_MESH_MODEL_OP_GEN_USER_PROPERTY_SET_UNACK:
                    ESP_LOGI(TAG, "user property id %d", param->value.set.user_property.property_id);
                    handle_gen_user_prop_msg(param->model, &param->ctx, NULL, &param->value.set.user_property);
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
