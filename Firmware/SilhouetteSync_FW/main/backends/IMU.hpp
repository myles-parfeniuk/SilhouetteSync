#pragma once 

//esp-idf includes
#include "driver/gpio.h"
#include "esp_log.h"

//in-house includes
#include "../Device.hpp"
#include "../defs/pin_definitions.hpp"
#include "BNO08x.hpp"

/**
*
* @brief IMU backend class.
*
* Class that manages onboard BNO085 IMU and its communication with the Device frontend, 
* can be set between different states (ex. sample, sleep, calibrate, etc..)
*/
class IMU {
    public:
    /**
     * @brief Constructs an IMU backend object. 
     * 
     * Creates object to manage SilhouetteSync's onboard IMU and communicate with Device frontend.  
     * 
     * @param d reference to Device frontend
     * @return void, nothing to return
     */
        IMU(Device &d);
    private:
        Device &d; ///<reference to device fontend to update with new samples, or receive state changes
        BNO08x imu; ///<imu driver object
        TaskHandle_t sample_task_hdl; ///<imu sampling task handle

     /**
     * @brief Launches imu sampling task.
     * 
     * This function is used to get around the fact xTaskCreate() from the freertos api requires a static task function.
     * 
     * To prevent having to write the imu sampling task from the context of a static function, this launches the sampling_task()
     * from the IMU object passed into xTaskCreate().
     * 
     * @param arg a void pointer to the IMU object from xTaskCreate() call
     * @return void, nothing to return
     */
        static void sampling_task_trampoline(void *arg);
    
    /**
     * @brief IMU sampling task.
     * 
     * This task updates the device model with new IMU data whenever it is received. 
     * It is notified and begins execution upon a switch to the sampling state.
     * It will execute until the state is switched out of the sampling state. 
     * 
     * @return void, nothing to return
     */
        void sampling_task();  
        static const constexpr char* TAG = "IMU"; ///<class tag, used in debug logs
};