//standard library includes
#include <stdio.h>
//esp-idf includes
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
//in-house includes
#include "Device.hpp"
#include "backends/NavSwitch.hpp"
#include "UI/UIManager.hpp"

extern "C" void app_main(void)
{
    Device d;
    NavSwitch nav_switch_driver(d);
    UIManager ui_driver(d);




    d.nav_switch.follow([](NavSwitchEvent switch_event)
    {
        switch(switch_event)
        {

            case NavSwitchEvent::enter_quick_press:
                ESP_LOGI("Main", "Enter Quick Press");
            break;

            case NavSwitchEvent::up_quick_press:
                ESP_LOGI("Main", "Up Quick Press");
            break;

            case NavSwitchEvent::down_quick_press:
                ESP_LOGI("Main", "Down Quick Press");
            break;

            case NavSwitchEvent::enter_long_press:
                ESP_LOGW("Main", "Enter Long Press");
            break;

            case NavSwitchEvent::up_long_press:
                ESP_LOGW("Main", "Up Long Press");
            break;

            case NavSwitchEvent::down_long_press:
                ESP_LOGW("Main", "Down Long Press");
            break;

            case NavSwitchEvent::enter_held:
                ESP_LOGI("Main", "Enter Held");
            break;

            case NavSwitchEvent::up_held:
                ESP_LOGI("Main", "Up Held");
            break;

            case NavSwitchEvent::down_held:
                ESP_LOGI("Main", "Down Held");
            break;

            case NavSwitchEvent::released:
                ESP_LOGE("Main", "Released");
            break;          

            default:
            break; 
        }
    });

    while(1)
    {
        
        vTaskDelay(10000/portTICK_PERIOD_MS);

    }

}
