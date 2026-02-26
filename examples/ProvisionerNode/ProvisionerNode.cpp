/**
 * @file ProvisionerNode.cpp
 * @brief Example: Combined provisioner + node (provisions others, sends & receives)
 *
 * This example demonstrates a DeziBot configured as a combined provisioner
 * and full node. The device:
 *   - Automatically discovers and provisions nearby unprovisioned DeziBots
 *   - Can send Generic OnOff commands to provisioned nodes (shake to toggle)
 *   - Can receive Generic OnOff commands and actuate LEDs
 *
 * No external provisioner (e.g. nRF Mesh app) is needed. This device
 * bootstraps the mesh network by itself.
 *
 * Usage:
 *   1. Flash this sketch to one DeziBot (the "coordinator").
 *   2. Flash the ServerOnly example to other DeziBots.
 *   3. Power them all on. The coordinator will automatically provision
 *      and configure the server nodes.
 *   4. Shake the coordinator to toggle all server LEDs.
 *
 * Note: No composition file is needed — begin() builds the merged
 * composition (provisioner + client + server models) internally.
 */

#include <Arduino.h>
#include <cstdint>
#include <Dezibot.h>
#include <DeziBotMesh.h>
#include <server/server_events.h>
#include <esp_log.h>

static const char *TAG = "main";

/// Broadcast address — send to all nodes in the mesh
const std::uint16_t TARGET_ALL = 0xFFFF;

bool commandState = false;
unsigned long lastShakeTime = 0;
const unsigned long SHAKE_DEBOUNCE_MS = 1000;

Dezibot dezibot = Dezibot();
DeziBotMesh dezimesh = DeziBotMesh();

/**
 * @brief Server event callback — handles incoming messages
 *
 * When another node sends a Generic OnOff Set to this device,
 * the LEDs are toggled accordingly.
 */
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
            if (evt->onoff_set.onoff) {
                dezibot.multiColorLight.setTopLeds(0x00FF00);
            } else {
                dezibot.multiColorLight.turnOffLed();
            }
            break;
        default:
            ESP_LOGI(TAG, "[srv evt] type=%d", (int)evt->type);
            break;
    }
}

void setup() {
    ESP_LOGI(TAG, "=== DeziBot Mesh Provisioner+Node Starting ===");

    dezibot.begin();
    ESP_LOGI(TAG, "Dezibot hardware initialized");

    if (!dezimesh.init()) {
        ESP_LOGE(TAG, "Mesh init failed");
        while (1) { delay(1000); }
    }
    ESP_LOGI(TAG, "Mesh stack initialized");

    // No composition_init() call needed — begin() builds it internally
    if (!dezimesh.beginProvisionerNode(onMeshServerEvent)) {
        ESP_LOGE(TAG, "Mesh provisioner+node init failed");
        while (1) { delay(1000); }
    }
    ESP_LOGI(TAG, "Mesh provisioner+node initialized");
    ESP_LOGI(TAG, "Device is self-provisioned at addr 0x0001");
    ESP_LOGI(TAG, "Scanning for unprovisioned devices...");
}

void loop() {
    // Shake to broadcast OnOff toggle to all provisioned nodes
    if (dezibot.motion.detection.isShaken(100, xAxis|yAxis|zAxis)
        && (millis() - lastShakeTime > SHAKE_DEBOUNCE_MS))
    {
        lastShakeTime = millis();
        commandState = !commandState;

        ESP_LOGI(TAG, "Sending OnOff(%s) to all nodes", commandState ? "ON" : "OFF");
        dezimesh.setOnOff(commandState, TARGET_ALL);
    }
    delay(10);
}
