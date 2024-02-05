#pragma once
//third party includes
//in-house includes
#include "defs/imu_defs.hpp"
#include "defs/pin_definitions.hpp"
#include "defs/connection_defs.hpp"
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

    Device(); 
    imu_t imu; ///<Device IMU frontend. 
    DataControl::CallAlways<LANConnectionStatus> lan_connection_status; ///<Current connection status of device to LAN

};