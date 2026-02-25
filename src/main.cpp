/**
 * @file Provisioner.cpp
 * @brief Example: Provisioner node (controls mesh)
 *
 * This example demonstrates a DeziBot configured as a provisioner
 * that controls the mesh network.
 */

#include <Arduino.h>
#include <cstdint>
#include <Dezibot.h>
#include <DeziBotMesh.h>
#include <esp_log.h>

static const char *TAG = "main";

Dezibot dezibot = Dezibot();
DeziBotMesh dezimesh = DeziBotMesh();

void setup() {
    ESP_LOGI(TAG, "=== DeziBot Mesh Provisioner Starting ===");

    dezibot.begin();
    ESP_LOGI(TAG, "Dezibot hardware initialized");

    if (!dezimesh.init()) {
        ESP_LOGE(TAG, "Mesh init failed");
        while (1) { delay(1000); }
    }
    ESP_LOGI(TAG, "Mesh stack initialized");

    if (!dezimesh.beginProvisioner()) {
        ESP_LOGE(TAG, "Mesh provisioner init failed");
        while (1) { delay(1000); }
    }
}

void loop() {
    delay(10);
}
