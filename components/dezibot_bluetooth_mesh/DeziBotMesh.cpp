/**
 * @file DeziBotMesh.cpp
 * @brief Arduino wrapper implementation for DeziBot Bluetooth Mesh library
 */

#include "DeziBotMesh.h"
#include "ble_mesh_bridge.h"

#include "esp_err.h"
#include "esp_log.h"

DeziBotMesh::DeziBotMesh()
    : _mode(MODE_NODE)
    , _initialized(false)
    , _meshInitialized(false) {
}

DeziBotMesh::~DeziBotMesh() {
    // Cleanup handled by ESP-IDF
}

bool DeziBotMesh::init() {
    if (_initialized) {
        return true;
    }

    esp_err_t err = mesh_bridge_pre_init();
    if (err != ESP_OK) {
        ESP_LOGE("DeziBotMesh", "pre_init failed: %d", err);
        return false;
    }

    /**
    err = mesh_bridge_bluetooth_init();
    if (err != ESP_OK) {
        ESP_LOGE("DeziBotMesh", "bluetooth_init failed: %d", err);
        return false;
    }
    **/

    _initialized = true;
    return true;
}

bool DeziBotMesh::beginClient() {
    if (!_initialized) {
        ESP_LOGE("DeziBotMesh", "Must call init() before beginClient()");
        return false;
    }

    if (_meshInitialized) {
        ESP_LOGW("DeziBotMesh", "Mesh already initialized");
        return true;
    }

    _mode = MODE_CLIENT_ONLY;
    
    // Initialize client only
    esp_err_t err = mesh_bridge_client_init();
    if (err != ESP_OK) {
        ESP_LOGE("DeziBotMesh", "ble_mesh_client_init failed: %d", err);
        return false;
    }

    _meshInitialized = true;
    ESP_LOGI("DeziBotMesh", "Mesh initialized as client-only");
    return true;
}

bool DeziBotMesh::beginServer(mesh_server_evt_cb_t serverCallback) {
    if (!_initialized) {
        ESP_LOGE("DeziBotMesh", "Must call init() before beginServer()");
        return false;
    }

    if (_meshInitialized) {
        ESP_LOGW("DeziBotMesh", "Mesh already initialized");
        return true;
    }

    _mode = MODE_SERVER_ONLY;

    // Initialize server
    esp_err_t err = mesh_bridge_server_init(serverCallback);
    if (err != ESP_OK) {
        ESP_LOGE("DeziBotMesh", "ble_mesh_server_init failed: %d", err);
        return false;
    }

    _meshInitialized = true;
    ESP_LOGI("DeziBotMesh", "Mesh initialized as server");
    return true;
}

bool DeziBotMesh::beginNode(mesh_server_evt_cb_t serverCallback) {
    if (!_initialized) {
        ESP_LOGE("DeziBotMesh", "Must call init() before beginNode()");
        return false;
    }

    if (_meshInitialized) {
        ESP_LOGW("DeziBotMesh", "Mesh already initialized");
        return true;
    }

    _mode = MODE_NODE;

    // Initialize node
    esp_err_t err = mesh_bridge_node_init(serverCallback);
    if (err != ESP_OK) {
        ESP_LOGE("DeziBotMesh", "mesh_bridge_node_init failed: %d", err);
        return false;
    }

    _meshInitialized = true;
    ESP_LOGI("DeziBotMesh", "Mesh initialized as node");
    return true;
}

esp_err_t DeziBotMesh::getOnOff(uint16_t addr, uint8_t elemIndex) {
    if (_mode == MODE_SERVER_ONLY) {
        ESP_LOGW("DeziBotMesh", "Cannot get onoff in server-only mode");
        return ESP_ERR_INVALID_STATE;
    }
    return mesh_bridge_client_get_onoff(addr, elemIndex);
}

esp_err_t DeziBotMesh::setOnOff(bool onoff, uint16_t addr, uint8_t elemIndex) {
    if (_mode == MODE_SERVER_ONLY) {
        ESP_LOGW("DeziBotMesh", "Cannot set onoff in server-only mode");
        return ESP_ERR_INVALID_STATE;
    }
    return mesh_bridge_client_set_onoff(onoff ? 1 : 0, addr, elemIndex);
}

esp_err_t DeziBotMesh::getLevel(uint16_t addr, uint8_t elemIndex) {
    if (_mode == MODE_SERVER_ONLY) {
        ESP_LOGW("DeziBotMesh", "Cannot get level in server-only mode");
        return ESP_ERR_INVALID_STATE;
    }
    return mesh_bridge_client_get_level(addr, elemIndex);
}

esp_err_t DeziBotMesh::setLevel(int16_t level, uint16_t addr, uint8_t elemIndex) {
    if (_mode == MODE_SERVER_ONLY) {
        ESP_LOGW("DeziBotMesh", "Cannot set level in server-only mode");
        return ESP_ERR_INVALID_STATE;
    }
    return mesh_bridge_client_set_level(level, addr, elemIndex);
}

esp_err_t DeziBotMesh::setDelta(int32_t delta, uint16_t addr, uint8_t elemIndex) {
    if (_mode == MODE_SERVER_ONLY) {
        ESP_LOGW("DeziBotMesh", "Cannot set delta in server-only mode");
        return ESP_ERR_INVALID_STATE;
    }
    return mesh_bridge_client_set_delta(delta, addr, elemIndex);
}

esp_err_t DeziBotMesh::setMove(int16_t move, uint16_t addr, uint8_t elemIndex) {
    if (_mode == MODE_SERVER_ONLY) {
        ESP_LOGW("DeziBotMesh", "Cannot set move in server-only mode");
        return ESP_ERR_INVALID_STATE;
    }
    return mesh_bridge_client_set_move(move, addr, elemIndex);
}

esp_err_t DeziBotMesh::getDefaultTransitionTime(uint16_t addr, uint8_t elemIndex) {
    if (_mode == MODE_SERVER_ONLY) {
        ESP_LOGW("DeziBotMesh", "Cannot get default transition time in server-only mode");
        return ESP_ERR_INVALID_STATE;
    }
    return mesh_bridge_client_get_default_transition_time(addr, elemIndex);
}

esp_err_t DeziBotMesh::setDefaultTransitionTime(uint8_t transitionTime, uint16_t addr, uint8_t elemIndex) {
    if (_mode == MODE_SERVER_ONLY) {
        ESP_LOGW("DeziBotMesh", "Cannot set default transition time in server-only mode");
        return ESP_ERR_INVALID_STATE;
    }
    return mesh_bridge_client_set_default_transition_time(transitionTime, addr, elemIndex);
}

esp_err_t DeziBotMesh::getOnPowerUp(uint16_t addr, uint8_t elemIndex) {
    if (_mode == MODE_SERVER_ONLY) {
        ESP_LOGW("DeziBotMesh", "Cannot get onpowerup in server-only mode");
        return ESP_ERR_INVALID_STATE;
    }
    return mesh_bridge_client_get_onpowerup(addr, elemIndex);
}

esp_err_t DeziBotMesh::setOnPowerUp(uint8_t power, uint16_t addr, uint8_t elemIndex) {
    if (_mode == MODE_SERVER_ONLY) {
        ESP_LOGW("DeziBotMesh", "Cannot set onpowerup in server-only mode");
        return ESP_ERR_INVALID_STATE;
    }
    return mesh_bridge_client_set_onpowerup(power, addr, elemIndex);
}

esp_err_t DeziBotMesh::getPowerLevel(uint16_t addr, uint8_t elemIndex) {
    if (_mode == MODE_SERVER_ONLY) {
        ESP_LOGW("DeziBotMesh", "Cannot get power level in server-only mode");
        return ESP_ERR_INVALID_STATE;
    }
    return mesh_bridge_client_get_power_level(addr, elemIndex);
}

esp_err_t DeziBotMesh::setPowerLevel(uint16_t power, uint16_t addr, uint8_t elemIndex) {
    if (_mode == MODE_SERVER_ONLY) {
        ESP_LOGW("DeziBotMesh", "Cannot set power level in server-only mode");
        return ESP_ERR_INVALID_STATE;
    }
    return mesh_bridge_client_set_power_level(power, addr, elemIndex);
}

esp_err_t DeziBotMesh::getPowerDefault(uint16_t addr, uint8_t elemIndex) {
    if (_mode == MODE_SERVER_ONLY) {
        ESP_LOGW("DeziBotMesh", "Cannot get power default in server-only mode");
        return ESP_ERR_INVALID_STATE;
    }
    return mesh_bridge_client_get_power_default(addr, elemIndex);
}

esp_err_t DeziBotMesh::setPowerDefault(uint16_t powerDefault, uint16_t addr, uint8_t elemIndex) {
    if (_mode == MODE_SERVER_ONLY) {
        ESP_LOGW("DeziBotMesh", "Cannot set power default in server-only mode");
        return ESP_ERR_INVALID_STATE;
    }
    return mesh_bridge_client_set_power_default(powerDefault, addr, elemIndex);
}

esp_err_t DeziBotMesh::getPowerRange(uint16_t addr, uint8_t elemIndex) {
    if (_mode == MODE_SERVER_ONLY) {
        ESP_LOGW("DeziBotMesh", "Cannot get power range in server-only mode");
        return ESP_ERR_INVALID_STATE;
    }
    return mesh_bridge_client_get_power_range(addr, elemIndex);
}

esp_err_t DeziBotMesh::setPowerRange(uint16_t min, uint16_t max, uint16_t addr, uint8_t elemIndex) {
    if (_mode == MODE_SERVER_ONLY) {
        ESP_LOGW("DeziBotMesh", "Cannot set power range in server-only mode");
        return ESP_ERR_INVALID_STATE;
    }
    return mesh_bridge_client_set_power_range(min, max, addr, elemIndex);
}

esp_err_t DeziBotMesh::getBattery(uint16_t addr, uint8_t elemIndex) {
    if (_mode == MODE_SERVER_ONLY) {
        ESP_LOGW("DeziBotMesh", "Cannot get battery in server-only mode");
        return ESP_ERR_INVALID_STATE;
    }
    return mesh_bridge_client_get_battery(addr, elemIndex);
}

esp_err_t DeziBotMesh::getLocGlobal(uint16_t addr, uint8_t elemIndex) {
    if (_mode == MODE_SERVER_ONLY) {
        ESP_LOGW("DeziBotMesh", "Cannot get global location in server-only mode");
        return ESP_ERR_INVALID_STATE;
    }
    return mesh_bridge_client_get_loc_global(addr, elemIndex);
}

esp_err_t DeziBotMesh::setLocGlobal(int32_t latitude, int32_t longitude, int16_t altitude, uint16_t addr, uint8_t elemIndex) {
    if (_mode == MODE_SERVER_ONLY) {
        ESP_LOGW("DeziBotMesh", "Cannot set global location in server-only mode");
        return ESP_ERR_INVALID_STATE;
    }
    return mesh_bridge_client_set_loc_global(latitude, longitude, altitude, addr, elemIndex);
}

esp_err_t DeziBotMesh::getLocLocal(uint16_t addr, uint8_t elemIndex) {
    if (_mode == MODE_SERVER_ONLY) {
        ESP_LOGW("DeziBotMesh", "Cannot get local location in server-only mode");
        return ESP_ERR_INVALID_STATE;
    }
    return mesh_bridge_client_get_loc_local(addr, elemIndex);
}

esp_err_t DeziBotMesh::setLocLocal(int16_t north, int16_t east, int16_t altitude, uint8_t floorNumber,
                                  uint16_t uncertainty, uint16_t addr, uint8_t elemIndex) {
    if (_mode == MODE_SERVER_ONLY) {
        ESP_LOGW("DeziBotMesh", "Cannot set local location in server-only mode");
        return ESP_ERR_INVALID_STATE;
    }
    return mesh_bridge_client_set_loc_local(north, east, altitude, floorNumber, uncertainty, addr, elemIndex);
}

esp_err_t DeziBotMesh::getUserProperties(uint16_t addr, uint8_t elemIndex) {
    if (_mode == MODE_SERVER_ONLY) {
        ESP_LOGW("DeziBotMesh", "Cannot get user properties in server-only mode");
        return ESP_ERR_INVALID_STATE;
    }
    return mesh_bridge_client_get_user_properties(addr, elemIndex);
}

esp_err_t DeziBotMesh::getUserProperty(uint16_t propertyId, uint16_t addr, uint8_t elemIndex) {
    if (_mode == MODE_SERVER_ONLY) {
        ESP_LOGW("DeziBotMesh", "Cannot get user property in server-only mode");
        return ESP_ERR_INVALID_STATE;
    }
    return mesh_bridge_client_get_user_property(propertyId, addr, elemIndex);
}

esp_err_t DeziBotMesh::setUserProperty(uint16_t propertyId, uint8_t *propertyValue, uint16_t propertyValueLen,
                                      uint16_t addr, uint8_t elemIndex) {
    if (_mode == MODE_SERVER_ONLY) {
        ESP_LOGW("DeziBotMesh", "Cannot set user property in server-only mode");
        return ESP_ERR_INVALID_STATE;
    }
    return mesh_bridge_client_set_user_property(propertyId, propertyValue, propertyValueLen, addr, elemIndex);
}

esp_err_t DeziBotMesh::getAdminProperties(uint16_t addr, uint8_t elemIndex) {
    if (_mode == MODE_SERVER_ONLY) {
        ESP_LOGW("DeziBotMesh", "Cannot get admin properties in server-only mode");
        return ESP_ERR_INVALID_STATE;
    }
    return mesh_bridge_client_get_admin_properties(addr, elemIndex);
}

esp_err_t DeziBotMesh::getAdminProperty(uint16_t propertyId, uint16_t addr, uint8_t elemIndex) {
    if (_mode == MODE_SERVER_ONLY) {
        ESP_LOGW("DeziBotMesh", "Cannot get admin property in server-only mode");
        return ESP_ERR_INVALID_STATE;
    }
    return mesh_bridge_client_get_admin_property(propertyId, addr, elemIndex);
}

esp_err_t DeziBotMesh::setAdminProperty(uint16_t propertyId, uint8_t adminAccess, uint8_t *propertyValue,
                                       uint16_t propertyValueLen, uint16_t addr, uint8_t elemIndex) {
    if (_mode == MODE_SERVER_ONLY) {
        ESP_LOGW("DeziBotMesh", "Cannot set admin property in server-only mode");
        return ESP_ERR_INVALID_STATE;
    }
    return mesh_bridge_client_set_admin_property(propertyId, adminAccess, propertyValue, propertyValueLen, addr, elemIndex);
}

esp_err_t DeziBotMesh::getManuProperties(uint16_t addr, uint8_t elemIndex) {
    if (_mode == MODE_SERVER_ONLY) {
        ESP_LOGW("DeziBotMesh", "Cannot get manufacturer properties in server-only mode");
        return ESP_ERR_INVALID_STATE;
    }
    return mesh_bridge_client_get_manu_properties(addr, elemIndex);
}

esp_err_t DeziBotMesh::getManuProperty(uint16_t propertyId, uint16_t addr, uint8_t elemIndex) {
    if (_mode == MODE_SERVER_ONLY) {
        ESP_LOGW("DeziBotMesh", "Cannot get manufacturer property in server-only mode");
        return ESP_ERR_INVALID_STATE;
    }
    return mesh_bridge_client_get_manu_property(propertyId, addr, elemIndex);
}

esp_err_t DeziBotMesh::setManuProperty(uint16_t propertyId, uint8_t adminAccess, uint16_t addr, uint8_t elemIndex) {
    if (_mode == MODE_SERVER_ONLY) {
        ESP_LOGW("DeziBotMesh", "Cannot set manufacturer property in server-only mode");
        return ESP_ERR_INVALID_STATE;
    }
    return mesh_bridge_client_set_manu_property(propertyId, adminAccess, addr, elemIndex);
}

esp_err_t DeziBotMesh::getClientProperties(uint16_t propertyId, uint16_t addr, uint8_t elemIndex) {
    if (_mode == MODE_SERVER_ONLY) {
        ESP_LOGW("DeziBotMesh", "Cannot get client properties in server-only mode");
        return ESP_ERR_INVALID_STATE;
    }
    return mesh_bridge_client_get_client_properties(propertyId, addr, elemIndex);
}

void DeziBotMesh::getDeviceUUID(uint8_t* uuid) {
    if (uuid != nullptr) {
        mesh_bridge_get_device_uuid(uuid);
    } else {
        ESP_LOGE("DeziBotMesh", "UUID buffer is null");
    }
}

// Global instance
DeziBotMesh DeziMesh;
