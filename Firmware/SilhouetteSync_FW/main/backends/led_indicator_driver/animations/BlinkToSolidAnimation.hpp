#pragma once

#include "../LedAnimation.hpp"

class BlinkToSolidAnimation: public LedAnimation
{
    public:
        BlinkToSolidAnimation(LedStrip &leds, EventGroupHandle_t &animation_event_group_hdl, uint8_t animation_bit, rgb_color_t color);
        void animation_core() override; 
    
    private:
        rgb_color_t color; 
        static const constexpr char *TAG = "BlinkToSolidAnimation";

};

