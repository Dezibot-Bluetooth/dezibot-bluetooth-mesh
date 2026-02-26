//
// Created by scholz on 2/26/26.
//


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

void prov_node_composition_init(void)
{
    prov_node_set_composition(&composition);
}