#include "LedIndicator.hpp"

LedIndicator::LedIndicator(Device &d):
d(d),
leds({1, pin_leds_di}),
led_anim_event_group_hdl(xEventGroupCreate()),
attempting_connection_animation(leds, led_anim_event_group_hdl, ATTEMPT_CONNECTION_ANIM_BIT, {true, true, false}, {2, 1, 1}),
calibration_animation(leds, led_anim_event_group_hdl, CALIBRATION_ANIM_BIT, {false, false, true}),
connected_animation(leds, led_anim_event_group_hdl, CONNECTED_ANIM_BIT, {0, 30, 0}),
failed_connection_animation(leds, led_anim_event_group_hdl, FAILED_CONNECTION_ANIM_BIT, {120, 0, 0})
{
    xTaskCreate(&led_anim_task_trampoline, "led_anim_task", 2048, this, 12, &led_anim_task_hdl); //launch led animation task
    active_animation = &attempting_connection_animation;

    d.lan_connection_status.follow([this](LANConnectionStatus new_status)
    {
        switch(new_status)
        {
            case LANConnectionStatus::failed_connection:
                ESP_LOGE(TAG, "FAILED"); 
                set_active_animation(&failed_connection_animation);
            break;

            case LANConnectionStatus::attempting_connection:
                ESP_LOGE(TAG, "ATTEMPTING");
                set_active_animation(&attempting_connection_animation);
            break;

            case LANConnectionStatus::connected:
                ESP_LOGE(TAG, "CONNECTED");
                set_active_animation(&connected_animation);
            break;

            default:

            break;

        }
    }, true);

    d.imu.state.follow([this](IMUState new_state)
    {
      
        if(new_state == IMUState::calibrate)
            set_active_animation(&calibration_animation);
        else
            calibration_animation.stop(); 

 
    }, true); 

    

}

void LedIndicator::led_anim_task_trampoline(void *arg)
{
    LedIndicator * local_led_indicator = (LedIndicator *)arg;

    local_led_indicator->led_anim_task(); 
}

void LedIndicator::led_anim_task()
{

    leds.clear_pixel_buffer();
    leds.write_pixel_buffer(); 
    
    while(1)
    {
        xEventGroupWaitBits(led_anim_event_group_hdl, ALL_ANIM_BITS, pdFALSE, pdFALSE, portMAX_DELAY);
        active_animation->animation_core();
    }
} 


void LedIndicator::set_active_animation(LedAnimation *new_animation)
{
    if(active_animation->get_animation_bit() != CALIBRATION_ANIM_BIT)
        active_animation->stop(); 

    active_animation = new_animation;
    active_animation->start(); 

}