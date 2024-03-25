#include "ChaseAnimation.hpp"

ChaseAnimation::ChaseAnimation(LedStrip& leds, uint8_t priority, rgb_color_t color, uint8_t update_delay_ms, bool direction)
    : LedAnimation(leds, priority)
    , color(color)
    , update_delay_ms(update_delay_ms)
    , direction(direction)
{
    esp_timer_create_args_t animation_timer_args = {.callback = &this->animation_timer_cb_trampoline, .arg = this, .name = "ChaseAnimation"};

    esp_timer_create(&animation_timer_args, &animation_timer_hdl);
}

void ChaseAnimation::start()
{
    build_pixel_buffer();
    esp_timer_start_periodic(animation_timer_hdl, update_delay_ms * 1000);
}

void ChaseAnimation::build_pixel_buffer()
{
    std::vector<rgb_color_t>& pixel_buffer = leds.get_pixel_buffer();

    for (int i = 0; i < pixel_buffer.size(); i++)
    {
        // assign color pattern here
        pixel_buffer.at(i).red = color.red * std::exp(-(i + 1));
        pixel_buffer.at(i).green = color.green * std::exp(-(i + 1));
        pixel_buffer.at(i).blue = color.blue * std::exp(-(i + 1));
    }
}

void ChaseAnimation::animation_timer_cb()
{
    std::vector<rgb_color_t>& pixel_buffer = leds.get_pixel_buffer();

    // rotate, false = clockwise, true = counter clockwise
    if (direction)
        std::rotate(pixel_buffer.begin(), pixel_buffer.begin() + 1, pixel_buffer.end());

    else
        std::rotate(pixel_buffer.rbegin(), pixel_buffer.rbegin() + 1, pixel_buffer.rend()); // shifts element one to the right

    esp_timer_restart(animation_timer_hdl, update_delay_ms * 1000);

    leds.write_pixel_buffer();
}