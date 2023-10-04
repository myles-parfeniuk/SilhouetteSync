#pragma once

//esp-idf includes
#include "esp_timer.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

//in-house includes
#include "../Device.hpp"
#include "U8G2Hal.hpp"
#include "../defs/pin_definitions.hpp"

class UIManager
{
    public:
        UIManager(Device &d); 

    private:
        Device &d; 
        void display_init();

        static const constexpr uint16_t oled_base_addr = 0x3C;
};