#pragma once
//third party includes
#include "u8g2.h"
//in-house includes
#include "defs/nav_switch_defs.hpp"
#include "DataControl.hpp"

class Device
{
    public:
    Device(); 
    u8g2_t display; ///<oled display
    DataControl::CallAlways<NavSwitchEvent> nav_switch; ///<navigation switch


};