#pragma once

// esp-idf includes
#include "nvs.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

class NVSManager
{
    public:
        static void initialize_NVS();
        static bool read_uint8_value(nvs_handle_t handle, const char* key, uint8_t *value);
        static bool write_uint8_value(nvs_handle_t handle, const char *key, uint8_t value);
};