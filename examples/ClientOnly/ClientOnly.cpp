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

const uint16_t TARGET_ALL = 0xFFFF;

bool commandState = false;
unsigned long lastShakeTime = 0;
const unsigned long SHAKE_DEBOUNCE_MS = 1000;

Dezibot dezibot = Dezibot();
DeziBotMesh dezimesh = DeziBotMesh();

void setup() {
    dezibot.begin();

    if (!dezimesh.init()) {
        while (1) { delay(1000); }
    }

    if (!dezimesh.beginClient()) {
        while (1) { delay(1000); }
    }
}

void loop() {
    // Check for shake gesture
    if (dezibot.motion.detection.isShaken() && (millis() - lastShakeTime > SHAKE_DEBOUNCE_MS)) {
        lastShakeTime = millis();
        commandState = !commandState;

        // Send OnOff command to all nodes
        Serial.print("Sending OnOff(");
        Serial.print(commandState ? "ON" : "OFF");
        Serial.println(") to all nodes");
        dezimesh.sendOnOff(commandState, TARGET_ALL);
    }
    delay(10);
}
