#include "PulseAnimation.hpp"

PulseAnimation::PulseAnimation(LedStrip& leds, uint8_t priority, color_channels_t color_channels, color_multiplier_t multipliers)
    : LedAnimation(leds, priority)
    , color_channels(color_channels)
    , multipliers(multipliers)
    , red(0)
    , green(0)
    , blue(0)
    , increasing_brightness(true)
{
    esp_timer_create_args_t animation_timer_args = {.callback = &this->animation_timer_cb_trampoline, .arg = this, .name = "PulseAnimationTimer"};

    esp_timer_create(&animation_timer_args, &animation_timer_hdl);
}

void PulseAnimation::start()
{
    red = 0;
    green = 0;
    blue = 0;
    increasing_brightness = true;
    esp_timer_start_periodic(animation_timer_hdl, 40000ULL);
}

void PulseAnimation::animation_timer_cb()
{
    leds.set_strip_color({(uint8_t) red, (uint8_t) green, (uint8_t) blue});
    leds.write_pixel_buffer();

    if (increasing_brightness)
    {
        if (color_channels.red)
            red += multipliers.red;

        if (color_channels.green)
            green += multipliers.green;

        if (color_channels.blue)
            blue += multipliers.blue;
    }
    else
    {
        if (color_channels.red)
            red -= multipliers.red;

        if (color_channels.green)
            green -= multipliers.green;

        if (color_channels.blue)
            blue -= multipliers.blue;
    }

    if (red >= 80 || green >= 80 || blue >= 80)
    {
        increasing_brightness = false;
    }
    else if (red < 0 || green < 0 || blue < 0)
    {
        increasing_brightness = true;
        red = 0;
        green = 0;
        blue = 0;
    }

    if (multipliers != (color_multiplier_t){1, 1, 1})
    {
        if (red > 40 || green > 40 || blue > 40)
            esp_timer_restart(animation_timer_hdl, 20000ULL);
        else if (red > 20 || green > 20 || blue > 20)
            esp_timer_restart(animation_timer_hdl, 30000ULL);
        else
            esp_timer_restart(animation_timer_hdl, 40000ULL);
    }
    else
    {
        if (red > 40 || green > 40 || blue > 40)
            esp_timer_restart(animation_timer_hdl, 10000ULL);
        else if (red > 20 || green > 20 || blue > 20)
            esp_timer_restart(animation_timer_hdl, 15000ULL);
        else
            esp_timer_restart(animation_timer_hdl, 20000ULL);
    };
}