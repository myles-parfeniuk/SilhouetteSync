#pragma once

// esp-idf includes
#include "driver/gpio.h"
#include "esp_log.h"

// in-house includes
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
class IMU
{
    public:
        /**
         * @brief Constructs an IMU backend object.
         *
         * Creates object to manage SilhouetteSync's onboard IMU and communicate with Device frontend.
         *
         * @param d reference to Device frontend
         * @return void, nothing to return
         */
        IMU(Device& d);

    private:
        Device& d;                 ///<reference to device fontend to update with new samples, or receive state changes
        BNO08x imu;                ///<imu driver object
        TaskHandle_t imu_task_hdl; ///<imu task handle
        EventGroupHandle_t imu_state_event_group_hdl;
        EventBits_t imu_state_bits;

        static const constexpr uint8_t CALIBRATION_STATE_BIT = BIT0;
        static const constexpr uint8_t SAMPLING_STATE_BIT = BIT1;
        static const constexpr uint8_t TARE_STATE_BIT = BIT2;
        static const constexpr uint8_t ALL_IMU_STATE_BITS = CALIBRATION_STATE_BIT | SAMPLING_STATE_BIT | TARE_STATE_BIT;

        static const constexpr char* TAG = "IMU"; ///<class tag, used in debug logs

        /**
         * @brief Launches imu sampling task.
         *
         * This function is used to get around the fact xTaskCreate() from the freertos api requires a static task function.
         *
         * To prevent having to write the imu sampling task from the context of a static function, this launches the imu_task()
         * from the IMU object passed into xTaskCreate().
         *
         * @param arg a void pointer to the IMU object from xTaskCreate() call
         * @return void, nothing to return
         */
        static void imu_task_trampoline(void* arg);

        /**
         * @brief IMU sampling task.
         *
         * Task responsible for handling sampling, passing of new data to the device model, and calibration.
         * The imu_event_group_hdl controls the flow of its execution.
         * @return void, nothing to return
         */
        void imu_task();

        void take_samples();

        void tare_imu();

        void calibrate_imu();

        bool calibration_routine();
};