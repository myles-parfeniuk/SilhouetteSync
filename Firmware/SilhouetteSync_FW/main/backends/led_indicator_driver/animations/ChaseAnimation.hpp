#pragma once

#include "../LedAnimation.hpp"
#include <math.h>
#include <vector>
#include <algorithm>

class ChaseAnimation : public LedAnimation
{
    public:
        ChaseAnimation(LedStrip& leds, uint8_t priority, rgb_color_t color, uint8_t update_delay_ms, bool direction);

    private:
        void animation_timer_cb() override;
        void start() override;
        void build_pixel_buffer();

        rgb_color_t color;  //RGB Value
        
        uint8_t update_delay_ms;
        bool direction;
        
        static const constexpr char* TAG = "ChaseAnimation";
};
