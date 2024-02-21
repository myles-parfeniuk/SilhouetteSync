#include "LedAnimation.hpp"

LedAnimation::LedAnimation(LedStrip &leds, EventGroupHandle_t &animation_event_group_hdl, uint8_t animation_bit):
leds(leds),
animation_event_group_hdl(animation_event_group_hdl),
animation_bit(animation_bit)
{

}

void LedAnimation::stop()
{
    xEventGroupClearBits(animation_event_group_hdl, animation_bit);
}

void LedAnimation::start()
{
    xEventGroupSetBits(animation_event_group_hdl, animation_bit);
}

uint8_t LedAnimation::get_animation_bit()
{
    return animation_bit; 
}