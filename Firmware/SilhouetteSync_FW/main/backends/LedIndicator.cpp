#include "LedIndicator.hpp"

LedIndicator::LedIndicator(Device &d):
d(d),
leds({1, pin_leds_di})
{
    d.lan_connection_status.follow([this](LANConnectionStatus new_status)
    {
        

        switch(new_status)
        {
            case LANConnectionStatus::failed_connection:
                ESP_LOGE(TAG, "FAILED"); 
                leds.set_strip_color({120, 0, 0});
                leds.write_pixel_buffer(); 
            break;

            case LANConnectionStatus::attempting_connection:
                ESP_LOGE(TAG, "ATTEMPTING");
                leds.set_strip_color({90, 40, 0});
                leds.write_pixel_buffer(); 
            break;

            case LANConnectionStatus::connected:
                ESP_LOGE(TAG, "CONNECTED");
                leds.set_strip_color({0, 120, 0});
                leds.write_pixel_buffer(); 
            break;

            default:

            break;

        }
    }, true);

}