// standard library includes
#include <stdio.h>
// esp-idf includes
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
// in-house includes
#include "Device.hpp"
#include "backends/IMU.hpp"
#include "defs/imu_defs.hpp"
#include "backends/wireless_communication/UDPServer.hpp"
#include "backends/led_indicator_driver/LedIndicator.hpp"
#include "backends/BatteryMonitor.hpp"
#include "backends/PowerManagement.hpp"

extern "C" void app_main(void)
{
    // create device model
    Device* d = new Device;
    // create backends
    //PowerManagement* power_manager = new PowerManagement(*d);
    LedIndicator* led_indicator_driver = new LedIndicator(*d);
    BatteryMonitor* battery_monitor = new BatteryMonitor(*d);
    IMU* imu_driver = new IMU(*d);
    UDPServer* server = new UDPServer(*d);

    // uncomment for live IMU data over serial for debug
    /*d.imu.data.follow([&d](imu_data_t new_data)
     {
         ESP_LOGI("Main", "x_heading: %.3f y_heading: %.3f z_heading: %.3f accuracy: %d",
         new_data.euler_heading.x,
         new_data.euler_heading.y,
         new_data.euler_heading.z,
         new_data.euler_heading.accuracy);

     }, true);*/

    // char task_run_time_stats[255];
    // char task_list[355];
    while (1)
    {
        vTaskDelay(5000 / portTICK_PERIOD_MS);

        // debugging: uncomment for free heap memory
        // ESP_LOGI("Main", "free heap size: %zu bytes", xPortGetFreeHeapSize());

        // debugging: uncomment for task cpu usage logs
        // vTaskGetRunTimeStats(task_run_time_stats);
        // ESP_LOGW("Main","%s\n", task_run_time_stats);

        // debugging:s uncomment for task stack usage logs
        // vTaskList(task_list);
        // ESP_LOGW("Main","%s\n", task_list);
    }
}
