#pragma once
// standard library includes
#include <string>
// in-house includes
#include "defs/imu_defs.hpp"
#include "defs/pin_definitions.hpp"
#include "defs/wireless_com_defs.hpp"
#include "defs/pwr_management_defs.hpp"
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
                DataControl::CallAlways<IMUState> state;          ///<The current state of the IMU (ie sample)
                DataControl::CallAlways<imu_data_t> data;         ///<The most recently received IMU data.
                DataControl::CallAlways<bool> calibration_status; ///<Whether or not the IMU has been calibrated.
        } imu_t;

        Device();
        DataControl::CallAlways<std::string> id;                            ///<Device id (unique MAC address).
        imu_t imu;                                                          ///<Device IMU frontend.
        DataControl::CallAlways<LANConnectionStatus> lan_connection_status; ///<current connection status of device to LAN
        DataControl::CallAlways<float> battery_voltage;                     ///<current battery voltage of device
        DataControl::CallAlways<PowerStates> power_state; ///< current power state of the device (ie battery powered, usb charging, usb fully charged)

        static const constexpr uint8_t HARDWARE_ID_OFFSET = 6;
        static const constexpr char* TAG = "Device"; ///<Device tag used in debug statements

    private:
        std::string get_hardware_id();
};