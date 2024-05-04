#include "SolidAnimation.hpp"

SolidAnimation::SolidAnimation(LedStrip& leds, uint8_t priority, rgb_color_t color)
    : LedAnimation(leds, priority)
    , color(color)
{
    esp_timer_create_args_t animation_timer_args = {.callback = &this->animation_timer_cb_trampoline, .arg = this, .name = "SolidAnimationTimer"};

    esp_timer_create(&animation_timer_args, &animation_timer_hdl);
}

void SolidAnimation::start()
{
    esp_timer_start_periodic(animation_timer_hdl, 10ULL);
}

void SolidAnimation::animation_timer_cb()
{
    static bool clear_strip = false;

    leds.clear_pixel_buffer();
    leds.write_pixel_buffer();
    vTaskDelay(2/portTICK_PERIOD_MS);
    leds.set_strip_color(color);
    leds.write_pixel_buffer();
    stop();
}