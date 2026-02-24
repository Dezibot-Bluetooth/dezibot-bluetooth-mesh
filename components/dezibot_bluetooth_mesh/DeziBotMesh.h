/**
 * @file DeziBotMesh.h
 * @brief Arduino wrapper for DeziBot Bluetooth Mesh library
 * 
 * This wrapper provides an Arduino-style C++ interface for the ESP-IDF
 * based Bluetooth Mesh networking functionality.
 */

#ifndef DEZIBOT_MESH_ARDUINO_H
#define DEZIBOT_MESH_ARDUINO_H

#include <Arduino.h>

/**
 * @brief Main DeziBot Bluetooth Mesh class
 * 
 * Provides a unified interface for Bluetooth Mesh networking with
 * flexible initialization modes (node, client-only, server-only).
 */
class DeziBotMesh {
public:
    /**
     * @brief Operating mode for the mesh node
     */
    enum Mode {
        MODE_NODE,         /// full, client + server
        MODE_CLIENT_ONLY,  /// client only
        MODE_SERVER_ONLY   /// server only
    };

    /**
     * @brief Constructor
     */
    DeziBotMesh();

    /**
     * @brief Destructor
     */
    ~DeziBotMesh();

    /**
     * @brief Initialize Bluetooth and pre-requisites
     * 
     * Call this once before begin(). Initializes NVS flash and Bluetooth stack.
     * 
     * @return true on success, false on failure
     */
    bool init();

    /**
     * @brief Initialize mesh node as client-only (send commands)
     * 
     * This initializes a node that can only send commands to other nodes.
     * Useful for remote controls or sensor nodes that only publish data.
     * 
     * @return true on success, false on failure
     */
    bool beginClient();

    /**
     * @brief Send Generic OnOff command to another node
     * 
     * @param onoff true=ON, false=OFF
     * @param addr Target unicast or group address
     */
    void sendOnOff(bool onoff, uint16_t addr);

    /**
     * @brief Get device UUID
     *
     * @param uuid Buffer to store UUID (must be 16 bytes)
     */
    void getDeviceUUID(uint8_t* uuid);

private:
    Mode _mode;
    bool _initialized;
    bool _meshInitialized;
};

// Global instance for convenience (optional - users can create their own)
extern DeziBotMesh DeziMesh;

#endif // DEZIBOT_MESH_ARDUINO_H
