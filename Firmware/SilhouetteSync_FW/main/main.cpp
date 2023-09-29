//standard library includes
#include <stdio.h>
#include "u8g2_esp32_hal.h"
//esp-idf includes
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
//in-house includes
#include "Device.hpp"
#include "backends/NavSwitch.hpp"

extern "C" void app_main(void)
{

    Device d;
    NavSwitch nav_switch_driver(d);

    while(1)
    {
        
        vTaskDelay(10000/portTICK_PERIOD_MS);

    }

}
