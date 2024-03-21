#pragma once

// standard library includes
#include <cmath>
#include <vector>
#include <iostream>
// esp-idf includes
#include "esp_log.h"

// in-house includes
#include "../../Device.hpp"
#include "../../defs/pin_definitions.hpp"
#include "../../defs/wireless_com_defs.hpp"
#include "LedStrip.hpp"
#include "LedAnimation.hpp"
#include "animations/PulseAnimation.hpp"
#include "animations/BlinkToSolidAnimation.hpp"

class LedIndicator
{
    public:
        LedIndicator(Device& d);

    private:
        enum class AnimationPriorities
        {
            lan_connection_status = 0, ///< show connection status animation group (failed_connection_animation, attempting_connection_animation)
            calibration_status,        ///< show calibration animation
            max
        };

        Device d;
        LedStrip leds;
        LedAnimation* active_anim;
        std::vector<LedAnimation*> queued_anims;
        PulseAnimation attempting_connection_anim;
        BlinkToSolidAnimation connected_anim;
        BlinkToSolidAnimation failed_connection_anim;
        PulseAnimation calibration_anim;
        void add_animation_to_queue(LedAnimation* new_anim);
        void remove_animation_from_queue(AnimationPriorities priority);
        void play_next_animation();

        static const constexpr uint8_t LED_COUNT = 4;
        static const constexpr char* TAG = "LedIndicator";
};