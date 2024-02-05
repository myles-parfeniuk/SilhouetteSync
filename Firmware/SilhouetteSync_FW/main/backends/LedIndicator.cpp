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
                leds.set_strip_color({120, 0, 0});
                leds.write_pixel_buffer(); 
            break;

            case LANConnectionStatus::attempting_connection:
                leds.set_strip_color({90, 40, 0});
                leds.write_pixel_buffer(); 
            break;

            case LANConnectionStatus::connected:
                leds.set_strip_color({0, 120, 0});
                leds.write_pixel_buffer(); 
            break;

            default:

            break;

        }
    }, true);

}