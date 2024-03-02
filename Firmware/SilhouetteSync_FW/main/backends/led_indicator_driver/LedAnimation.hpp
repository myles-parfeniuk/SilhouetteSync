#pragma once

// esp-idf includes
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
// in-house includes
#include "LedStrip.hpp"

class LedAnimation
{
    public:
        LedAnimation(LedStrip& leds, uint8_t priority);
        void stop();
        uint8_t get_priority();
        static void animation_timer_cb_trampoline(void* arg);
        virtual void start() = 0;
        virtual void animation_timer_cb() = 0;

    protected:
        LedStrip& leds;
        uint8_t priority;
        esp_timer_handle_t animation_timer_hdl;
        static const constexpr char* TAG = "LedAnimation";
};