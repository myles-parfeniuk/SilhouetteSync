#pragma once
// standard library includes
#include <string>
// in-house includes
#include "defs/imu_defs.hpp"
#include "defs/pin_defs.hpp"
#include "defs/wireless_com_defs.hpp"
#include "defs/pwr_management_defs.hpp"
#include "defs/switch_defs.hpp"
#include "DataControl.hpp"

/**
 *
 * @brief Device frontend class.
 *
 * Class that represents SilhouetteSync as a whole, facilitates communication between backends (ie SwitchDriver and PowerManager).
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
        DataControl::CallAlways<std::string> id;                            ///<Device id (unique randomized hardware ID burnt into fuse block)
        imu_t imu;                                                          ///<Device IMU frontend.
        DataControl::CallAlways<LANConnectionStatus> lan_connection_status; ///<Current connection status of device to LAN
        DataControl::CallAlways<float> battery_voltage;                     ///<Current battery voltage of device as sampled by ADC
        DataControl::CallAlways<PowerSourceStates>
                power_source_state; ///<Current power source state of the device (ie battery powered, usb charging, usb fully charged)
        DataControl::CallAlways<PowerStates>
                power_state; ///<Current power state of the device (ie low powered (no wifi, buck disabled), normal operation
        DataControl::CallAlways<SwitchEvents> user_sw; ///<Most recent user input event on switch

        static const constexpr uint8_t HARDWARE_ID_OFFSET = 6; ///<Hardware ID offset (used for loading hardware ID from fuse block 3)
        static const constexpr char* TAG = "Device";           ///<Device tag used in debug statements

    private:
        /**
         * @brief Reads custom device hardware ID from fuse block 3.
         *
         * To the esp32 of each respective bracelet, custom hardware IDs have been burnt to an efuse block using a python script.
         * These hardware IDs are strings in the flavor of two random words compounded together with a hyphen. They are used by
         * the unity software suite to differentiate body locations.
         *
         * @return void, nothing to return
         */
        std::string get_hardware_id();
};