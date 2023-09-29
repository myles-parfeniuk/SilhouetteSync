#pragma once
#include "defs/nav_switch_events.hpp"
#include "DataControl.hpp"

class Device
{
    public:
    Device(); 
    DataControl::CallAlways<NavSwitchEvent> nav_switch; ///<navigation switch


};