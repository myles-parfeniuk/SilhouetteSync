//standard library includes
#include <stdio.h>
//esp-idf includes
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
//in-house includes
#include "Device.hpp"
#include "backends/IMU.hpp"
//#include "backends/TCPServer.hpp"
#include "backends/UDPServer.hpp"
#include "backends/LedIndicator.hpp"

extern "C" void app_main(void)
{
    Device d; //create device model
    IMU imu_driver(d); //initialize IMU driver
    LedIndicator led_indicator_driver(d);
    UDPServer server(d);
    //TCPServer server(d);

    d.imu.state.set(IMUState::sample);

    //uncomment for live IMU data over serial for debug
   /*d.imu.data.follow([&d](imu_data_t new_data)
    {
        ESP_LOGI("Main", "x_heading: %.3f y_heading: %.3f z_heading: %.3f accuracy: %d", 
        new_data.euler_heading.x, 
        new_data.euler_heading.y, 
        new_data.euler_heading.z, 
        new_data.euler_heading.accuracy);

    }, true);*/

    char task_run_time_stats[255];
    char task_list[255];
    while(1)
    {
        vTaskDelay(5000/portTICK_PERIOD_MS);

        //debugging: uncomment for free heap memory 
        //ESP_LOGI("Main", "free heap size: %zu bytes", xPortGetFreeHeapSize());

        //debugging: uncomment for task cpu usage logs
        //vTaskGetRunTimeStats(task_run_time_stats);
        //ESP_LOGW("Main","%s\n", task_run_time_stats);

        //debugging: uncomment for task stack usage logs
        //vTaskList(task_list);
        //ESP_LOGW("Main","%s\n", task_list);


    }

}

