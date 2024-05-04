#pragma once

#include "../LedAnimation.hpp"
#include "../../../Device.hpp"
#include "math.h"

class StatusBarAnimation : public LedAnimation
{
    public:
        StatusBarAnimation(LedStrip& leds, uint8_t priority, Device &d, bool dim);
    private:
        Device &d;
        bool dim;
        void animation_timer_cb() override;
        void start() override;
        static constexpr rgb_color_t color = {5, 0, 15};
        static constexpr rgb_color_t color_dim = {1, 0, 3};
        static const constexpr char* TAG = "SolidAnimation";
};
