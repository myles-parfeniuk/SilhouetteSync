#pragma once 

//standard library includes
#include <cmath> 
//esp-idf includes
#include "esp_log.h"

//in-house includes
#include "../Device.hpp"
#include "../defs/pin_definitions.hpp"
#include "../defs/connection_defs.hpp"
#include "LedStrip.hpp"

class LedIndicator
{
    public:
        LedIndicator(Device &d);
    
    private:
        Device d; 
        LedStrip leds; 
        TaskHandle_t led_anim_task_hdl;
        EventGroupHandle_t led_anim_event_group_hdl;
        static void led_anim_task_trampoline(void *arg); 
        void led_anim_task(); 
        void attempting_connection_anim(); 
        void failed_connection_anim(); 
        void connected_anim(); 
        void calibration_anim(); 
        static const constexpr uint8_t ATTEMPT_CONNECTION_ANIM_BIT = BIT0; 
        static const constexpr uint8_t FAILED_CONNECTION_ANIM_BIT = BIT1; 
        static const constexpr uint8_t CONNECTED_ANIM_BIT = BIT2; 
        static const constexpr uint8_t CALIBRATION_ANIM_BIT = BIT3; 
        static const constexpr uint8_t ALL_ANIM_BITS = ATTEMPT_CONNECTION_ANIM_BIT|FAILED_CONNECTION_ANIM_BIT|CONNECTED_ANIM_BIT|CALIBRATION_ANIM_BIT;


        static const constexpr char *TAG = "LedIndicator";
};