#include "BlinkToSolidAnimation.hpp"

BlinkToSolidAnimation::BlinkToSolidAnimation(LedStrip &leds, EventGroupHandle_t &animation_event_group_hdl, uint8_t animation_bit, rgb_color_t color):
LedAnimation(leds, animation_event_group_hdl, animation_bit),
color(color)
{

}

void BlinkToSolidAnimation::animation_core()
{
    for(int i = 0; i < 5; i++)
    {
        leds.set_strip_color(color);
        leds.write_pixel_buffer(); 
        vTaskDelay(100/portTICK_PERIOD_MS);
        leds.clear_pixel_buffer(); 
        leds.write_pixel_buffer(); 
        vTaskDelay(100/portTICK_PERIOD_MS);
    }

    leds.set_strip_color(color);
    leds.write_pixel_buffer();

    stop(); 

}