#include <stdio.h>
#include "wireless_communication/UDPServer.hpp"

extern "C" void app_main(void)
{
    UDPServer server;

    while(1)
    {
        vTaskDelay(10000/portTICK_PERIOD_MS);
    }
}
