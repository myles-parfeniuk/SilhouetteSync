//standard library includes
#include <stdio.h>
//esp-idf includes
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
//in-house includes
#include "Device.hpp"
#include "backends/IMU.hpp"
#include "backends/TCPServer.hpp"
#include "backends/LedIndicator.hpp"

extern "C" void app_main(void)
{
    Device d; //create device model

    LedIndicator led_indicator_driver(d);
    TCPServer server(d);
    IMU imu_driver(d); //initialize IMU driver

    d.imu.state.set(IMUState::calibrate);
    d.imu.state.set(IMUState::sample);

   /*d.imu.data.follow([&d](imu_data_t new_data)
    {
        ESP_LOGI("Main", "x_heading: %.3f y_heading: %.3f z_heading: %.3f accuracy: %d", 
        new_data.euler_heading.x, 
        new_data.euler_heading.y, 
        new_data.euler_heading.z, 
        new_data.euler_heading.accuracy);

    }, true);*/


    while(1)
    {
        vTaskDelay(10000/portTICK_PERIOD_MS);
    }

}
