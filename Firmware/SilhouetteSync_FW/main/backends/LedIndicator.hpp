#pragma once 

//esp-idf includes
#include "esp_log.h"

//in-house includes
#include "../Device.hpp"
#include "../defs/pin_definitions.hpp"
#include "../defs/connection_defs.hpp"
#include "LedStrip.hpp"

class LedIndicator
{
    public:
        LedIndicator(Device &d);
    
    private:
        Device d; 
        LedStrip leds; 
};