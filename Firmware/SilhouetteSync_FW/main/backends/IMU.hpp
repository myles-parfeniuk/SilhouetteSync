#pragma once 

//esp-idf includes
#include "driver/gpio.h"
#include "esp_log.h"

//third-party includes
#include "BNO055ESP32.h"

//in-house includes
#include "../Device.hpp"
#include "../defs/pin_definitions.hpp"

class IMU {
    public:
        IMU(Device &d);
    private:
        Device &d; 
        BNO055 imu_unit;
        TaskHandle_t sample_task_hdl;
        bool take_samples;
        bool calibrating;  
        static void sampling_task_trampoline(void *nav_switch);
        void sampling_task();  
        void calibrate_imu(); 
        static const constexpr char* TAG = "IMU"; ///<class tag, used in debug logs
};