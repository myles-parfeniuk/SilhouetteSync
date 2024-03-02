#pragma once

#include <array>
#include "../LedAnimation.hpp"

typedef struct
{
        bool red;
        bool green;
        bool blue;

} color_channels_t;

typedef struct color_multiplier_t
{
        uint8_t red;
        uint8_t green;
        uint8_t blue;

        inline bool operator!=(const color_multiplier_t& rhs) const
        {
            return (red != rhs.red) || (green != rhs.green) || (blue != rhs.blue);
        }

        inline bool operator==(const color_multiplier_t& rhs) const
        {
            return (red == rhs.red) && (green == rhs.green) && (blue == rhs.blue);
        }

} color_multiplier_t;

class PulseAnimation : public LedAnimation
{
    public:
        PulseAnimation(LedStrip& leds, uint8_t priority, color_channels_t color_channels, color_multiplier_t multipliers = {1, 1, 1});

    private:
        void animation_timer_cb() override;
        void start() override;
        color_channels_t color_channels;
        color_multiplier_t multipliers;
        int16_t red;
        int16_t green;
        int16_t blue;
        bool increasing_brightness;
        static const constexpr char* TAG = "PulseAnimation";
};
