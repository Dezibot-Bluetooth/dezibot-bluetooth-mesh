#ifndef DEZIBOT_BLUETOOTH_MESH_SERVER_EVENTS_H
#define DEZIBOT_BLUETOOTH_MESH_SERVER_EVENTS_H

#include <stdint.h>

typedef enum {
    MESH_EVT_ONOFF_GET,
    MESH_EVT_ONOFF_SET,
    MESH_EVT_LEVEL_GET,
    MESH_EVT_LEVEL_SET,
    MESH_EVT_DELTA_SET,
    MESH_EVT_MOVE_SET,
    MESH_EVT_DEF_TRANS_TIME_GET,
    MESH_EVT_DEF_TRANS_TIME_SET,
    MESH_EVT_ONPOWERUP_GET,
    MESH_EVT_ONPOWERUP_SET,
    MESH_EVT_POWER_DEFAULT_GET,
    MESH_EVT_POWER_DEFAULT_SET,
    MESH_EVT_POWER_LEVEL_GET,
    MESH_EVT_POWER_LEVEL_SET,
    MESH_EVT_POWER_LAST_GET,
    MESH_EVT_POWER_RANGE_GET,
    MESH_EVT_POWER_RANGE_SET,
    MESH_EVT_POWER_BATTERY_GET,
    MESH_EVT_LOC_GLOBAL_GET,
    MESH_EVT_LOC_GLOBAL_SET,
    MESH_EVT_LOC_LOCAL_GET,
    MESH_EVT_LOC_LOCAL_SET,
    MESH_EVT_CLIENT_PROPERTIES_GET,
    MESH_EVT_ADMIN_PROPERTIES_GET,
    MESH_EVT_ADMIN_PROPERTY_GET,
    MESH_EVT_ADMIN_PROPERTY_SET,
    MESH_EVT_MANU_PROPERTIES_GET,
    MESH_EVT_MANU_PROPERTY_GET,
    MESH_EVT_MANU_PROPERTY_SET,
    MESH_EVT_USER_PROPERTIES_GET,
    MESH_EVT_USER_PROPERTY_GET,
    MESH_EVT_USER_PROPERTY_SET,
} mesh_server_evt_type_t;

typedef struct {
    mesh_server_evt_type_t type;

    union {
        struct {
            uint8_t onoff;
        } onoff_set;
        struct {
            int16_t level;
        } level_set;
        struct {
            int16_t delta_level;
        } delta_set;
        struct {
            int16_t move_level;
        } move_set;
        struct {
            uint8_t trans_time;
        } def_trans_time_set;
        struct {
            uint8_t onpowerup;
        } onpowerup_set;
        struct{
            uint16_t power;
        } power_default_set;
        struct {
            uint16_t power;
        } power_level_set;
        struct {
            uint16_t range_min;
            uint16_t range_max;
        } power_range_set;
        struct {
            int32_t global_latitude;
            int32_t global_longitude;
            int32_t global_altitude;
        } loc_global_set;
        struct {
            int16_t local_north;
            int16_t local_east;
            int16_t local_altitude;
            int8_t floor_number;
            uint8_t uncertainty;
        } loc_local_set;
        struct {
            uint16_t property_id;
            uint8_t user_access;
        } admin_property_set, manu_property_set, user_property_set;
    };
} mesh_server_evt_t;

typedef void (*mesh_server_evt_cb_t)(const mesh_server_evt_t *evt);

#endif //DEZIBOT_BLUETOOTH_MESH_SERVER_EVENTS_H