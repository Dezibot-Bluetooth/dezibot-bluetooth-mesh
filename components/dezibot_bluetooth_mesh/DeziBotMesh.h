/**
 * @file DeziBotMesh.h
 * @brief Arduino wrapper for DeziBot Bluetooth Mesh library
 * 
 * This wrapper provides an Arduino-style C++ interface for the ESP-IDF
 * based Bluetooth Mesh networking functionality.
 */

#ifndef DEZIBOT_MESH_ARDUINO_H
#define DEZIBOT_MESH_ARDUINO_H

#include <stdint.h>
#include <esp_err.h>
#include <server/server_events.h>

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
    bool init(char *device_name);

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
     * @brief Initialize mesh node as server-only (receive commands and store data)
     *
     * This initializes a node that can only receives commands and stores data.
     *
     * @return true on success, false on failure
     */
    bool beginServer(mesh_server_evt_cb_t serverCallback);

    /**
     * @brief Initialize mesh node as with clients and servers
     *
     * This initializes a node which can both send commands and receive commands.
     * This is the most common mode for general-purpose nodes.
     *
     * @return true on success, false on failure
     */
    bool beginNode(mesh_server_evt_cb_t serverCallback);

    /**
     * @brief Get Generic OnOff state from another node
     *
     * Sends a Generic OnOff Get to the target address.
     *
     * @param addr Target unicast or group address
     * @param elemIndex Local element index to use (default: 0)
     *
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t getOnOff(uint16_t addr, uint8_t elemIndex = 0);

    /**
     * @brief Set Generic OnOff state on another node
     *
     * Sends a Generic OnOff Set Unacknowledged to the target address.
     *
     * @param onoff true=ON, false=OFF
     * @param addr Target unicast or group address
     * @param elemIndex Local element index to use (default: 0)
     *
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t setOnOff(bool onoff, uint16_t addr, uint8_t elemIndex = 0);

    /**
     * @brief Get Generic Level state from another node
     *
     * Sends a Generic Level Get to the target address.
     *
     * @param addr Target unicast or group address
     * @param elemIndex Local element index to use (default: 0)
     *
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t getLevel(uint16_t addr, uint8_t elemIndex = 0);

    /**
     * @brief Set Generic Level state on another node
     *
     * Sends a Generic Level Set Unacknowledged to the target address.
     *
     * @param level Target level value
     * @param addr Target unicast or group address
     * @param elemIndex Local element index to use (default: 0)
     *
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t setLevel(int16_t level, uint16_t addr, uint8_t elemIndex = 0);

    /**
     * @brief Send Generic Delta Set Unacknowledged to another node
     *
     * @param delta Delta level value
     * @param addr Target unicast or group address
     * @param elemIndex Local element index to use (default: 0)
     *
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t setDelta(int32_t delta, uint16_t addr, uint8_t elemIndex = 0);

    /**
     * @brief Send Generic Move Set Unacknowledged to another node
     *
     * @param move Move delta level value
     * @param addr Target unicast or group address
     * @param elemIndex Local element index to use (default: 0)
     *
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t setMove(int16_t move, uint16_t addr, uint8_t elemIndex = 0);

    /**
     * @brief Get Generic Default Transition Time from another node
     *
     * @param addr Target unicast or group address
     * @param elemIndex Local element index to use (default: 0)
     *
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t getDefaultTransitionTime(uint16_t addr, uint8_t elemIndex = 0);

    /**
     * @brief Set Generic Default Transition Time on another node
     *
     * @param transitionTime Transition time value
     * @param addr Target unicast or group address
     * @param elemIndex Local element index to use (default: 0)
     *
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t setDefaultTransitionTime(uint8_t transitionTime, uint16_t addr, uint8_t elemIndex = 0);

    /**
     * @brief Get Generic OnPowerUp state from another node
     *
     * @param addr Target unicast or group address
     * @param elemIndex Local element index to use (default: 0)
     *
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t getOnPowerUp(uint16_t addr, uint8_t elemIndex = 0);

    /**
     * @brief Set Generic OnPowerUp state on another node
     *
     * @param power OnPowerUp value
     * @param addr Target unicast or group address
     * @param elemIndex Local element index to use (default: 0)
     *
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t setOnPowerUp(uint8_t power, uint16_t addr, uint8_t elemIndex = 0);

    /**
     * @brief Get Generic Power Level state from another node
     *
     * @param addr Target unicast or group address
     * @param elemIndex Local element index to use (default: 0)
     *
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t getPowerLevel(uint16_t addr, uint8_t elemIndex = 0);

    /**
     * @brief Set Generic Power Level on another node
     *
     * @param power Target power value
     * @param addr Target unicast or group address
     * @param elemIndex Local element index to use (default: 0)
     *
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t setPowerLevel(uint16_t power, uint16_t addr, uint8_t elemIndex = 0);

    /**
     * @brief Get Generic Power Default from another node
     *
     * @param addr Target unicast or group address
     * @param elemIndex Local element index to use (default: 0)
     *
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t getPowerDefault(uint16_t addr, uint8_t elemIndex = 0);

    /**
     * @brief Set Generic Power Default on another node
     *
     * @param powerDefault Target power default value
     * @param addr Target unicast or group address
     * @param elemIndex Local element index to use (default: 0)
     *
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t setPowerDefault(uint16_t powerDefault, uint16_t addr, uint8_t elemIndex = 0);

    /**
     * @brief Get Generic Power Range from another node
     *
     * @param addr Target unicast or group address
     * @param elemIndex Local element index to use (default: 0)
     *
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t getPowerRange(uint16_t addr, uint8_t elemIndex = 0);

    /**
     * @brief Set Generic Power Range on another node
     *
     * @param min Minimum power value
     * @param max Maximum power value
     * @param addr Target unicast or group address
     * @param elemIndex Local element index to use (default: 0)
     *
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t setPowerRange(uint16_t min, uint16_t max, uint16_t addr, uint8_t elemIndex = 0);

    /**
     * @brief Get Generic Battery state from another node
     *
     * Sends a Generic Battery Get to the target address.
     *
     * @param addr Target unicast or group address
     * @param elemIndex Local element index to use (default: 0)
     *
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t getBattery(uint16_t addr, uint8_t elemIndex = 0);

    /**
     * @brief Get Generic Global Location state from another node
     *
     * @param addr Target unicast or group address
     * @param elemIndex Local element index to use (default: 0)
     *
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t getLocGlobal(uint16_t addr, uint8_t elemIndex = 0);

    /**
     * @brief Set Generic Global Location state on another node
     *
     * @param latitude Global latitude value
     * @param longitude Global longitude value
     * @param altitude Global altitude value
     * @param addr Target unicast or group address
     * @param elemIndex Local element index to use (default: 0)
     *
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t setLocGlobal(int32_t latitude, int32_t longitude, int16_t altitude, uint16_t addr, uint8_t elemIndex = 0);

    /**
     * @brief Get Generic Local Location state from another node
     *
     * @param addr Target unicast or group address
     * @param elemIndex Local element index to use (default: 0)
     *
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t getLocLocal(uint16_t addr, uint8_t elemIndex = 0);

    /**
     * @brief Set Generic Local Location state on another node
     *
     * @param north Local north value
     * @param east Local east value
     * @param altitude Local altitude value
     * @param floorNumber Local floor number value
     * @param uncertainty Uncertainty value
     * @param addr Target unicast or group address
     * @param elemIndex Local element index to use (default: 0)
     *
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t setLocLocal(int16_t north, int16_t east, int16_t altitude, uint8_t floorNumber,
                          uint16_t uncertainty, uint16_t addr, uint8_t elemIndex = 0);

    /**
     * @brief Get list of Generic User Property IDs from another node
     *
     * @param addr Target unicast or group address
     * @param elemIndex Local element index to use (default: 0)
     *
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t getUserProperties(uint16_t addr, uint8_t elemIndex = 0);

    /**
     * @brief Get a Generic User Property value from another node
     *
     * @param propertyId Property ID
     * @param addr Target unicast or group address
     * @param elemIndex Local element index to use (default: 0)
     *
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t getUserProperty(uint16_t propertyId, uint16_t addr, uint8_t elemIndex = 0);

    /**
     * @brief Set a Generic User Property value on another node
     *
     * Sends a Generic User Property Set Unacknowledged.
     *
     * @param propertyId Property ID
     * @param propertyValue Pointer to the property value buffer
     * @param propertyValueLen Length of the property value buffer
     * @param addr Target unicast or group address
     * @param elemIndex Local element index to use (default: 0)
     *
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t setUserProperty(uint16_t propertyId, uint8_t *propertyValue, uint16_t propertyValueLen,
                              uint16_t addr, uint8_t elemIndex = 0);

    /**
     * @brief Get list of Generic Admin Property IDs from another node
     *
     * @param addr Target unicast or group address
     * @param elemIndex Local element index to use (default: 0)
     *
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t getAdminProperties(uint16_t addr, uint8_t elemIndex = 0);

    /**
     * @brief Get a Generic Admin Property value from another node
     *
     * @param propertyId Property ID
     * @param addr Target unicast or group address
     * @param elemIndex Local element index to use (default: 0)
     *
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t getAdminProperty(uint16_t propertyId, uint16_t addr, uint8_t elemIndex = 0);

    /**
     * @brief Set a Generic Admin Property value on another node
     *
     * Sends a Generic Admin Property Set Unacknowledged.
     *
     * @param propertyId Property ID
     * @param adminAccess Admin access value
     * @param propertyValue Pointer to the property value buffer
     * @param propertyValueLen Length of the property value buffer
     * @param addr Target unicast or group address
     * @param elemIndex Local element index to use (default: 0)
     *
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t setAdminProperty(uint16_t propertyId, uint8_t adminAccess, uint8_t *propertyValue,
                               uint16_t propertyValueLen, uint16_t addr, uint8_t elemIndex = 0);

    /**
     * @brief Get list of Generic Manufacturer Property IDs from another node
     *
     * @param addr Target unicast or group address
     * @param elemIndex Local element index to use (default: 0)
     *
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t getManuProperties(uint16_t addr, uint8_t elemIndex = 0);

    /**
     * @brief Get a Generic Manufacturer Property value from another node
     *
     * @param propertyId Property ID
     * @param addr Target unicast or group address
     * @param elemIndex Local element index to use (default: 0)
     *
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t getManuProperty(uint16_t propertyId, uint16_t addr, uint8_t elemIndex = 0);

    /**
     * @brief Set a Generic Manufacturer Property value on another node
     *
     * Sends a Generic Manufacturer Property Set Unacknowledged.
     *
     * @param propertyId Property ID
     * @param adminAccess Admin access value
     * @param addr Target unicast or group address
     * @param elemIndex Local element index to use (default: 0)
     *
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t setManuProperty(uint16_t propertyId, uint8_t adminAccess, uint16_t addr, uint8_t elemIndex = 0);

    /**
     * @brief Get list of Generic Client Property IDs from another node
     *
     * @param propertyId Client property ID
     * @param addr Target unicast or group address
     * @param elemIndex Local element index to use (default: 0)
     *
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t getClientProperties(uint16_t propertyId, uint16_t addr, uint8_t elemIndex = 0);

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
