#pragma once

// standard library includes
#include <cmath>
#include <vector>
// esp-idf includes
#include "esp_log.h"

// in-house includes
#include "../../Device.hpp"
#include "../../defs/pin_definitions.hpp"
#include "../../defs/connection_defs.hpp"
#include "LedStrip.hpp"
#include "LedAnimation.hpp"
#include "animations/PulseAnimation.hpp"
#include "animations/BlinkToSolidAnimation.hpp"

class LedIndicator
{
    public:
        LedIndicator(Device& d);

    private:
        Device d;
        LedStrip leds;
        LedAnimation* active_animation;
        std::vector<LedAnimation*> queued_animations;
        PulseAnimation attempting_connection_animation;
        BlinkToSolidAnimation connected_animation;
        BlinkToSolidAnimation failed_connection_animation;
        PulseAnimation calibration_animation;
        void add_animation_to_queue(LedAnimation* new_animation);
        void remove_animation_from_queue(uint8_t priority);
        void play_next_animation();

        static const constexpr uint8_t LED_COUNT = 1;
        static const constexpr char* TAG = "LedIndicator";
};