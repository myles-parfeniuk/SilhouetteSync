#include "LedIndicator.hpp"

LedIndicator::LedIndicator(Device &d):
d(d),
leds({1, pin_leds_di}),
led_anim_event_group_hdl(xEventGroupCreate())
{
    xTaskCreate(&led_anim_task_trampoline, "led_anim_task", 2048, this, 12, &led_anim_task_hdl); //launch tcp server task

    d.lan_connection_status.follow([this](LANConnectionStatus new_status)
    {
        

        switch(new_status)
        {
            case LANConnectionStatus::failed_connection:
                ESP_LOGE(TAG, "FAILED"); 
                xEventGroupSetBits(led_anim_event_group_hdl, FAILED_CONNECTION_ANIM);
            break;

            case LANConnectionStatus::attempting_connection:
                ESP_LOGE(TAG, "ATTEMPTING");
                xEventGroupSetBits(led_anim_event_group_hdl, ATTEMPT_CONNECTION_ANIM);
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

void LedIndicator::led_anim_task_trampoline(void *arg)
{
    LedIndicator * local_led_indicator = (LedIndicator *)arg;

    local_led_indicator->led_anim_task(); 
}

void LedIndicator::led_anim_task()
{
    EventBits_t animation_bits;

    leds.clear_pixel_buffer();
    leds.write_pixel_buffer(); 
    
    while(1)
    {
        animation_bits = xEventGroupWaitBits(led_anim_event_group_hdl,
        ATTEMPT_CONNECTION_ANIM | FAILED_CONNECTION_ANIM,
        pdTRUE,
        pdFALSE,
        portMAX_DELAY);

        if(animation_bits & FAILED_CONNECTION_ANIM)
        {
            for(int i = 0; i < 5; i++)
            {
                leds.set_strip_color({120, 0, 0});
                leds.write_pixel_buffer(); 
                vTaskDelay(100/portTICK_PERIOD_MS);
                leds.clear_pixel_buffer(); 
                leds.write_pixel_buffer(); 
                vTaskDelay(100/portTICK_PERIOD_MS);
            }

            leds.set_strip_color({120, 0, 0});
            leds.write_pixel_buffer(); 

        }
        else if(animation_bits & ATTEMPT_CONNECTION_ANIM)
        {
            attempting_connection_anim(); 
        }
    }
} 

void LedIndicator::attempting_connection_anim()
{
        
    leds.set_strip_color({90, 40, 0});
    leds.write_pixel_buffer(); 
}