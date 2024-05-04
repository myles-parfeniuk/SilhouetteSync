#pragma once

// standard library includes
#include <cmath>
#include <vector>
#include <iostream>
// esp-idf includes
#include "esp_log.h"

// in-house includes
#include "../../Device.hpp"
#include "../../defs/pin_defs.hpp"
#include "../../defs/wireless_com_defs.hpp"
#include "LedStrip.hpp"
#include "LedAnimation.hpp"
#include "animations/PulseAnimation.hpp"
#include "animations/BlinkToSolidAnimation.hpp"
#include "animations/ChaseAnimation.hpp"
#include "animations/SolidAnimation.hpp"
#include "animations/StatusBarAnimation.hpp"

class LedIndicator
{
    public:
        LedIndicator(Device& d);

    private:
        enum class AnimationPriorities
        {
            lan_connection_status = 0, ///< show connection status animation group (failed_connection_animation, attempting_connection_animation)
            calibration_status,        ///< show calibration animation
            battery_indicator,         ///< show battery indicator animation
            low_power_mode,            ///< show low power mode animation
            shutdown,                  ///< show shutdown animation
            boot,                      ///< show boot animation
            max
        };

        Device d;
        LedStrip leds;
        bool display_battery_indicator; 
        LedAnimation* active_anim;
        std::vector<LedAnimation*> queued_anims;
        PulseAnimation attempting_connection_anim;
        SolidAnimation connected_anim;
        BlinkToSolidAnimation failed_connection_anim;
        PulseAnimation calibration_anim;
        ChaseAnimation shutdown_anim;
        ChaseAnimation boot_anim;
        StatusBarAnimation low_power_anim;
        StatusBarAnimation battery_indicator_anim;

        void add_animation_to_queue(LedAnimation* new_anim);
        void remove_animation_from_queue(AnimationPriorities priority);
        void play_next_animation();

        static const constexpr uint8_t LED_COUNT = 4;
        static const constexpr char* TAG = "LedIndicator";
};