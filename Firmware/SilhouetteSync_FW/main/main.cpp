//standard library includes
#include <stdio.h>
//esp-idf includes
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
//in-house includes
#include "Device.hpp"
#include "backends/NavSwitch.hpp"
#include "backends/IMU.hpp"
#include "UI/UIManager.hpp"
#

extern "C" void app_main(void)
{
    Device d; //create device model
    NavSwitch nav_switch_driver(d); 
    UIManager ui_driver(d); 
    IMU imu_driver(d);

    while(1)
    {
        
        vTaskDelay(10000/portTICK_PERIOD_MS);

    }

}
