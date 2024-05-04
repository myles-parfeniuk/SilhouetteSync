#pragma once

#include "../LedAnimation.hpp"

class SolidAnimation : public LedAnimation
{
    public:
        SolidAnimation(LedStrip& leds, uint8_t priority, rgb_color_t color);

    private:
        void animation_timer_cb() override;
        void start() override;
        rgb_color_t color;
        static const constexpr char* TAG = "SolidAnimation";
};
