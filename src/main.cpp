/**
* @file ClientOnly.cpp
 * @brief Example: Client-only node (sends commands)
 *
 * This example demonstrates a DeziBot configured as a client-only node
 * that sends commands to other nodes in the mesh network.
 */

#include <Arduino.h>
#include <cstdint>
#include <Dezibot.h>
#include <DeziBotMesh.h>
#include <esp_log.h>
#include "client_composition.h"

static const char *TAG = "main";

const std::uint16_t TARGET_ALL = 0xFFFF;

bool commandState = false;
unsigned long lastShakeTime = 0;
const unsigned long SHAKE_DEBOUNCE_MS = 1000;

Dezibot dezibot = Dezibot();
DeziBotMesh dezimesh = DeziBotMesh();

void setup() {
    ESP_LOGI(TAG, "=== DeziBot Mesh Client Starting ===");

    dezibot.begin();
    ESP_LOGI(TAG, "Dezibot hardware initialized");

    if (!dezimesh.init()) {
        ESP_LOGE(TAG, "Mesh init failed");
        while (1) { delay(1000); }
    }
    ESP_LOGI(TAG, "Mesh stack initialized");

    client_composition_init();
    ESP_LOGI(TAG, "Set client composition");

    if (!dezimesh.beginClient("ClientOnly")) {
        ESP_LOGE(TAG, "Mesh client init failed");
        while (1) { delay(1000); }
    }
}

void loop() {
    if (dezibot.motion.detection.isShaken(100, xAxis|yAxis|zAxis) && (millis() - lastShakeTime > SHAKE_DEBOUNCE_MS)) {
        lastShakeTime = millis();
        commandState = !commandState;

        ESP_LOGI(TAG, "Sending OnOff(%s) to all nodes", commandState ? "ON" : "OFF");
        dezimesh.setOnOff(commandState, TARGET_ALL);
    }
    delay(10);
}
