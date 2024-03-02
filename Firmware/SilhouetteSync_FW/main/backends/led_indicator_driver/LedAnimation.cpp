#include "LedAnimation.hpp"

LedAnimation::LedAnimation(LedStrip& leds, uint8_t priority)
    : leds(leds)
    , priority(priority)
{
}

void LedAnimation::stop()
{
    esp_timer_stop(animation_timer_hdl);
}

uint8_t LedAnimation::get_priority()
{
    return priority;
}

void LedAnimation::animation_timer_cb_trampoline(void* arg)
{
    LedAnimation* local_animation = (LedAnimation*) arg;
    local_animation->animation_timer_cb();
}