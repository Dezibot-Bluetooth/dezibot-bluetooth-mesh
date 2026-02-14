/**
 * @file DeziBotMesh.cpp
 * @brief Arduino wrapper implementation for DeziBot Bluetooth Mesh library
 */

#include "DeziBotMesh.h"

extern "C" {
    #include "../lib/common.h"
    #include "../lib/init.h"
    #include "../lib/bluetooth.h"
    #include "../lib/client.h"
}

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

    esp_err_t err = pre_init();
    if (err != ESP_OK) {
        ESP_LOGE("DeziBotMesh", "pre_init failed: %d", err);
        return false;
    }

    err = bluetooth_init();
    if (err != ESP_OK) {
        ESP_LOGE("DeziBotMesh", "bluetooth_init failed: %d", err);
        return false;
    }

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
    esp_err_t err = ble_mesh_client_init();
    if (err != ESP_OK) {
        ESP_LOGE("DeziBotMesh", "ble_mesh_client_init failed: %d", err);
        return false;
    }

    _meshInitialized = true;
    ESP_LOGI("DeziBotMesh", "Mesh initialized as client-only");
    return true;
}

void DeziBotMesh::sendOnOff(bool onoff, uint16_t addr) {
    if (_mode == MODE_NODE) {
        // TODO
    } else if (_mode == MODE_CLIENT_ONLY) {
        ble_mesh_client_send(onoff ? 1 : 0, addr);
    } else {
        ESP_LOGW("DeziBotMesh", "Cannot send in server-only mode");
    }
}

void DeziBotMesh::getDeviceUUID(uint8_t* uuid) {
    if (uuid != nullptr) {
        ble_mesh_get_dev_uuid(uuid);
    } else {
        ESP_LOGE("DeziBotMesh", "UUID buffer is null");
    }
}

// Global instance
DeziBotMesh DeziMesh;
