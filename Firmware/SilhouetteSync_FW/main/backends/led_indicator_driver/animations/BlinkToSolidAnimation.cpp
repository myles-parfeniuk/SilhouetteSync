#include "BlinkToSolidAnimation.hpp"

BlinkToSolidAnimation::BlinkToSolidAnimation(LedStrip& leds, uint8_t priority, rgb_color_t color)
    : LedAnimation(leds, priority)
    , color(color)
    , blink_count(0)
{
    esp_timer_create_args_t animation_timer_args = {
        .callback = &this->animation_timer_cb_trampoline, .arg = this, .name = "BlinkToSolidAnimationTimer"};

    esp_timer_create(&animation_timer_args, &animation_timer_hdl);
}

void BlinkToSolidAnimation::start()
{
    blink_count = 0;
    esp_timer_start_periodic(animation_timer_hdl, 100000ULL);
}

void BlinkToSolidAnimation::animation_timer_cb()
{
    static bool clear_strip = false;

    if (blink_count < 5)
    {
        if (blink_count == 0)
            clear_strip = false;

        if (clear_strip)
        {
            leds.clear_pixel_buffer();
            leds.write_pixel_buffer();
        }
        else
        {
            leds.set_strip_color(color);
            leds.write_pixel_buffer();
            blink_count++;
        }
    }
    else
    {
        stop();
    }

    clear_strip = !clear_strip;
}