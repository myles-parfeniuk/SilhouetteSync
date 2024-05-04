#include "StatusBarAnimation.hpp"

StatusBarAnimation::StatusBarAnimation(LedStrip& leds, uint8_t priority, Device& d, bool dim)
    : LedAnimation(leds, priority)
    , d(d)
    , dim(dim)
{

    esp_timer_create_args_t animation_timer_args = {.callback = &this->animation_timer_cb_trampoline, .arg = this, .name = "StatusBarAnimationTimer"};
    esp_timer_create(&animation_timer_args, &animation_timer_hdl);
}

void StatusBarAnimation::start()
{
    float percent = (float) d.battery.soc_percentage.get();
    int max_i = (int) round((percent / 100.0) * 4.0f);
    rgb_color_t local_color;

    if (dim)
        local_color = color_dim;
    else
        local_color = color;

    leds.clear_pixel_buffer();
    leds.write_pixel_buffer();

    vTaskDelay(1 / portTICK_PERIOD_MS);

    for (int i = 0; i < max_i; i++)
        leds.set_pixel_color(i, local_color);

    leds.write_pixel_buffer();

    esp_timer_start_periodic(animation_timer_hdl, 5000000ULL);
}

void StatusBarAnimation::animation_timer_cb()
{
    float percent = (float) d.battery.soc_percentage.get();
    int max_i = (int) round((percent / 100.0) * 4.0f);
    rgb_color_t local_color;

    if (dim)
        local_color = color_dim;
    else
        local_color = color;

    leds.clear_pixel_buffer();
    leds.write_pixel_buffer();

    vTaskDelay(1 / portTICK_PERIOD_MS);

    for (int i = 0; i < max_i; i++)
        leds.set_pixel_color(i, local_color);

    leds.write_pixel_buffer();
}