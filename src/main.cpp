/**
* @file ServerOnly.cpp
 * @brief Example: Server-only node (receives commands)
 *
 * This example demonstrates a DeziBot configured as a server-only node.
 * The server uses a simple OnOff server that can receive commands from other nodes.
 */

#include <Arduino.h>
#include <Dezibot.h>
#include <DeziBotMesh.h>
#include <server/server_events.h>
#include "server_composition.h"

static const char *TAG = "main";

// Prevent Arduino's initArduino() from releasing BT controller memory.
// Without this, esp_bt_controller_mem_release() runs before setup(),
// and the later nimble_port_init() crashes on freed memory.
extern "C" bool btInUse() {
    return true;
}

Dezibot dezibot = Dezibot();
DeziBotMesh dezimesh = DeziBotMesh();

static void onMeshServerEvent(const mesh_server_evt_t *evt)
{
    if (!evt) {
        return;
    }

    switch (evt->type) {
    case MESH_EVT_ONOFF_GET:
        ESP_LOGI(TAG, "[srv evt] ONOFF_GET");
        break;
    case MESH_EVT_ONOFF_SET:
        ESP_LOGI(TAG, "[srv evt] ONOFF_SET: onoff=%u", evt->onoff_set.onoff);
        break;
    default:
        ESP_LOGI(TAG, "[srv evt] type=%d", (int)evt->type);
        break;
    }
}

void setup() {
    ESP_LOGI(TAG, "=== DeziBot Mesh Client Starting ===");

    dezibot.begin();
    ESP_LOGI(TAG, "Dezibot hardware initialized");

    if (!dezimesh.init()) {
        ESP_LOGE(TAG, "Mesh init failed");
        while (1) { delay(1000); }
    }
    ESP_LOGI(TAG, "Mesh stack initialized");

    server_composition_init();
    ESP_LOGI(TAG, "Set server composition");

    if (!dezimesh.beginServer(onMeshServerEvent))
    {
        ESP_LOGE(TAG, "Mesh server init failed");
        while (1) { delay(1000); }
    }
    ESP_LOGI(TAG, "Mesh server initialized");
}

void loop() {
    delay(10);
}
