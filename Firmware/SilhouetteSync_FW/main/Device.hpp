#pragma once
//third party includes
#include "u8g2.h"
//in-house includes
#include "defs/imu_defs.hpp"
#include "defs/pin_definitions.hpp"
#include "defs/nav_switch_defs.hpp"
#include "DataControl.hpp"

/**
*
* @brief Device frontend class.
*
* Class that represents SilhouetteSync as a whole, facilitates communication between backends (ie NavSwitch driver)
* and frontends (ie UIManager).
*/
class Device
{
    public:
    
    /** 
    *   @brief  Represents BNO08x IMU unit. 
    */
    typedef struct 
    {
        DataControl::CallAlways<IMUState> state; ///<The current state of the IMU (ie sample)
        DataControl::CallAlways<imu_data_t> data; ///<The most recently received IMU data.
        DataControl::CallAlways<bool> calibration_status; ///<Whether or not the IMU has been calibrated. 
    } imu_t;

    /** 
    *   @brief  Represents navigation switch. 
    */
    typedef struct nav_switch_t
    {
        DataControl::CallAlways<nav_switch_evt_t> up; ///<Most recent up input event (ie quick-press, long-press, etc...)
        DataControl::CallAlways<nav_switch_evt_t> down; ///<Most recent down input event (ie quick-press, long-press, etc...)
        DataControl::CallAlways<nav_switch_evt_t> enter; ///<Most recent enter input event (ie quick-press, long-press, etc...)
    } nav_switch_t; 


    Device(); 
    u8g2_t display; ///<u8g2 display struct used for u8g2 OLED drawing functions
    imu_t imu; ///<Device IMU frontend. 
    nav_switch_t nav_switch; ///<Device navigation switch frontend
        



};