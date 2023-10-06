#pragma once
//third party includes
#include "u8g2.h"
//in-house includes
#include "defs/nav_switch_defs.hpp"
#include "defs/imu_defs.hpp"
#include "BNO055ESP32.h"
#include "DataControl.hpp"

class Device
{
    public:
    
    typedef struct {
        DataControl::CallAlways<IMUState> state;
        DataControl::CallAlways<bno055_vector_t> vector;
        DataControl::CallAlways<bool> calibration_status; 
    } imu_t;


    Device(); 
    u8g2_t display; ///<oled display
    DataControl::CallAlways<NavSwitchEvent> nav_switch; ///<navigation switch
    imu_t imu;



};