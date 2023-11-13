#pragma once
//third party includes
#include "u8g2.h"
//in-house includes
#include "defs/nav_switch_defs.hpp"
#include "defs/imu_defs.hpp"
#include "DataControl.hpp"

class Device
{
    public:
    
    typedef struct {
        DataControl::CallAlways<IMUState> state;
        DataControl::CallAlways<imu_data_t> data;
        DataControl::CallAlways<bool> calibration_status; 
    } imu_t;


    Device(); 
    u8g2_t display; ///<oled display
    DataControl::CallAlways<NavSwitchEvent> nav_switch; ///<navigation switch
    imu_t imu;



};