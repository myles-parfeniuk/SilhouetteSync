#pragma once

#include "../LedAnimation.hpp"

class BlinkToSolidAnimation : public LedAnimation
{
    public:
        BlinkToSolidAnimation(LedStrip& leds, uint8_t priority, rgb_color_t color);

    private:
        void animation_timer_cb() override;
        void start() override;
        rgb_color_t color;
        uint8_t blink_count;
        static const constexpr char* TAG = "BlinkToSolidAnimation";
};
