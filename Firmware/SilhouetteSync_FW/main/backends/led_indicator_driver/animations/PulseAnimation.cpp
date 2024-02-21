#include "PulseAnimation.hpp"

PulseAnimation::PulseAnimation(LedStrip &leds, EventGroupHandle_t &animation_event_group_hdl, uint8_t animation_bit, color_channels_t color_channels, color_multiplier_t multipliers):
LedAnimation(leds, animation_event_group_hdl, animation_bit),
color_channels(color_channels),
multipliers(multipliers)
{   

}

void PulseAnimation::animation_core()
{
    EventBits_t animation_bits;
    int16_t red = 0;
    int16_t green = 0; 
    int16_t blue = 0; 
    bool increasing_brightness = true;

    do
    {

        leds.set_strip_color({(uint8_t)red, (uint8_t)green, (uint8_t)blue});
        leds.write_pixel_buffer(); 

        if(increasing_brightness)
        {
            if(color_channels.red)
                red+= multipliers.red;

            if(color_channels.green)
                green+= multipliers.green;

            if(color_channels.blue)
                blue+= multipliers.blue;
        }
        else
        {
            if(color_channels.red)
                red-= multipliers.red;

            if(color_channels.green)
                green-= multipliers.green;

            if(color_channels.blue)
                blue-= multipliers.blue;
        }

        if(red >= 80 || green >= 80 || blue >= 80)
        {
            increasing_brightness = false;
        }
        else if(red < 0 || green < 0 || blue < 0)
        {
            increasing_brightness = true; 
            red = 0;
            green = 0; 
            blue = 0; 
        }
        
        if(multipliers != (color_multiplier_t){1, 1, 1})
        {
            if(red > 40 || green > 40 || blue > 40)
                vTaskDelay(20/portTICK_PERIOD_MS);
            else if(red > 20 || green > 20 || blue > 20)
                vTaskDelay(30/portTICK_PERIOD_MS);
            else
                vTaskDelay(40/portTICK_PERIOD_MS);
        }
        else
        {
            if(red > 40 || green > 40 || blue > 40)
                vTaskDelay(10/portTICK_PERIOD_MS);
            else if(red > 20 || green > 20 || blue > 20)
                vTaskDelay(15/portTICK_PERIOD_MS);
            else
                vTaskDelay(20/portTICK_PERIOD_MS);
        }

        animation_bits = xEventGroupWaitBits(animation_event_group_hdl, animation_bit, pdFALSE, pdFALSE, 0);

    } while (animation_bits & animation_bit);
}