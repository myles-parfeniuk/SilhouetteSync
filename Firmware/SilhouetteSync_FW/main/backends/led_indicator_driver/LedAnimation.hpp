#pragma once 

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "LedStrip.hpp"

class LedAnimation
{
    public:
        LedAnimation(LedStrip &leds, EventGroupHandle_t &animation_event_group_hdl, uint8_t animation_bit);
        void start();
        void stop();
        uint8_t get_animation_bit();
        virtual void animation_core() = 0;
    protected:
        LedStrip &leds;
        EventGroupHandle_t &animation_event_group_hdl; 
        uint8_t animation_bit; 
        static const constexpr char *TAG = "LedIndicator";
};