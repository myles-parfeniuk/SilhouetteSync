//standard library includes
#include <stdio.h>
//esp-idf includes
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
//in-house includes
#include "Device.hpp"
#include "backends/IMU.hpp"
#include "backends/NavSwitch.hpp"
#include "UI/UIManager.hpp"

extern "C" void app_main(void)
{
    Device d; //create device model
    UIManager ui_driver(d); //initialize OLED menu system
    IMU imu_driver(d); //initialize IMU driver
    NavSwitch nav_switch(d); //initialize nav switch driver

    while(1)
    {
        
        vTaskDelay(10000/portTICK_PERIOD_MS);

    }

}
