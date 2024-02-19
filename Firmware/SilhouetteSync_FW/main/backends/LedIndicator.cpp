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
                xEventGroupClearBits(led_anim_event_group_hdl, ALL_ANIM_BITS & (~CALIBRATION_ANIM_BIT));
                xEventGroupSetBits(led_anim_event_group_hdl, FAILED_CONNECTION_ANIM_BIT);
            break;

            case LANConnectionStatus::attempting_connection:
                ESP_LOGE(TAG, "ATTEMPTING");
                xEventGroupClearBits(led_anim_event_group_hdl, ALL_ANIM_BITS & (~CALIBRATION_ANIM_BIT));
                xEventGroupSetBits(led_anim_event_group_hdl, ATTEMPT_CONNECTION_ANIM_BIT);
            break;

            case LANConnectionStatus::connected:
                ESP_LOGE(TAG, "CONNECTED");
                xEventGroupClearBits(led_anim_event_group_hdl, ALL_ANIM_BITS & (~CALIBRATION_ANIM_BIT));
                xEventGroupSetBits(led_anim_event_group_hdl, CONNECTED_ANIM_BIT);
            break;

            default:

            break;

        }
    }, true);

    d.imu.state.follow([this](IMUState new_state)
    {
      
        if(new_state == IMUState::calibrate)
        {
            xEventGroupSetBits(led_anim_event_group_hdl, CALIBRATION_ANIM_BIT);
        }
        else
        {
            xEventGroupClearBits(led_anim_event_group_hdl, CALIBRATION_ANIM_BIT);
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
        ALL_ANIM_BITS,
        pdFALSE,
        pdFALSE,
        portMAX_DELAY);
        
        if(animation_bits & CALIBRATION_ANIM_BIT)
            calibration_anim(); 
        else if(animation_bits & FAILED_CONNECTION_ANIM_BIT)
            failed_connection_anim(); 
        else if(animation_bits & ATTEMPT_CONNECTION_ANIM_BIT)
            attempting_connection_anim(); 
        else if(animation_bits & CONNECTED_ANIM_BIT)
            connected_anim(); 
    }
} 

void LedIndicator::attempting_connection_anim()
{
    EventBits_t animation_bits;
    uint8_t red = 0;
    uint8_t green = 0; 
    bool increasing_brightness = true; 

    do
    {

        leds.set_strip_color({red, green, 0});
        leds.write_pixel_buffer(); 

        if(increasing_brightness)
        {
            red+= 2;
            green++; 
        }
        else
        {
            red-= 2;
            green--;
        }

        if(red >= 80)
        {
            increasing_brightness = false;
        }
        else if(red <= 0)
        {
            increasing_brightness = true; 
        }

        if(red > 40)
            vTaskDelay(20/portTICK_PERIOD_MS);
        else if(red >20)
            vTaskDelay(30/portTICK_PERIOD_MS);
        else
            vTaskDelay(40/portTICK_PERIOD_MS);

        animation_bits = xEventGroupWaitBits(led_anim_event_group_hdl,
        ALL_ANIM_BITS,
        pdFALSE,
        pdFALSE,
        portMAX_DELAY);

    } while (animation_bits & ATTEMPT_CONNECTION_ANIM_BIT);
    

}

void LedIndicator::failed_connection_anim()
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

    xEventGroupClearBits(led_anim_event_group_hdl, ALL_ANIM_BITS & (~CALIBRATION_ANIM_BIT));
}

void LedIndicator::connected_anim()
{
    for(int i = 0; i < 5; i++)
    {
        leds.set_strip_color({0, 30, 0});
        leds.write_pixel_buffer(); 
        vTaskDelay(100/portTICK_PERIOD_MS);
        leds.clear_pixel_buffer(); 
        leds.write_pixel_buffer(); 
        vTaskDelay(100/portTICK_PERIOD_MS);
    }

    leds.set_strip_color({0, 30, 0});
    leds.write_pixel_buffer(); 

    xEventGroupClearBits(led_anim_event_group_hdl, ALL_ANIM_BITS & (~CALIBRATION_ANIM_BIT));
}

void LedIndicator::calibration_anim()
{
    EventBits_t animation_bits;
    uint8_t blue = 0; 
    bool increasing_brightness = true; 

    do
    {
        leds.set_strip_color({0, 0, blue});
        leds.write_pixel_buffer(); 

        if(increasing_brightness)
            blue++; 
        else
            blue--; 

        if(blue >= 80)
        {
            increasing_brightness = false;
        }
        else if(blue <= 0)
        {
            increasing_brightness = true; 
        }

        if(blue > 40)
            vTaskDelay(20/portTICK_PERIOD_MS);
        else if(blue >20)
            vTaskDelay(30/portTICK_PERIOD_MS);
        else
            vTaskDelay(40/portTICK_PERIOD_MS);

        animation_bits = xEventGroupWaitBits(led_anim_event_group_hdl,
        ALL_ANIM_BITS,
        pdFALSE,
        pdFALSE,
        portMAX_DELAY);

    } while (animation_bits & CALIBRATION_ANIM_BIT);
    

}