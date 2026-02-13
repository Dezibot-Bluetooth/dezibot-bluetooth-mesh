#ifndef DEZIBOT_BLUETOOTH_MESH_CLIENT_H
#define DEZIBOT_BLUETOOTH_MESH_CLIENT_H

#include "common.h"

esp_err_t ble_mesh_client_init(void);

// Generic OnOff Client
void ble_mesh_client_send(uint8_t val, uint16_t addr);

// Generic Level Client
void ble_mesh_client_send_level(int16_t level, uint16_t addr);

// Generic Default Transition Time Client
void ble_mesh_client_send_default_transition_time(uint8_t transition_time, uint16_t addr);

// Generic Power Level Client
void ble_mesh_client_send_power_level(uint16_t power, uint16_t addr);

// Generic Battery Client
void ble_mesh_client_send_battery(uint8_t battery_level, uint16_t addr);

// Generic Location Client
void ble_mesh_client_send_location(uint32_t latitude, uint32_t longitude, int16_t altitude, uint16_t addr);

// Generic Property Client
void ble_mesh_client_send_property(uint16_t property_id, uint8_t *property_value, 
                                    uint16_t property_value_len, uint16_t addr);

#endif //DEZIBOT_BLUETOOTH_MESH_CLIENT_H
