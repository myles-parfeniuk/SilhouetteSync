#pragma once
// standard library includes
#include <stdio.h>
#include <math.h>
#include <cmath>
// esp-idf includes
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/soc_caps.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
// in-house includes
#include "../Device.hpp"
#include "../defs/pin_defs.hpp"

/**
 * @brief IMU backend class.
 *
 * Class that monitors onboard battery voltage with ADC and reports it to device fontend  model.
 */
class BatteryMonitor
{
    public:
        /**
         * @brief Constructs an IMU backend object.
         *
         * Creates object to manage SilhouetteSync's onboard battery and communicate voltage level with Device frontend.
         *
         * @param d reference to Device frontend
         * @return void, nothing to return
         */
        BatteryMonitor(Device& d);

    private:
        /**
         * @brief Initializes adc for monitoring battery voltage.
         *
         * @return void, nothing to return
         */
        void adc_init();

        /**
         * @brief Calibrates ADC channel that is being utilized to monitor battery voltage.
         * @return void, nothing to return
         */
        bool adc_calibrate_ch(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t* out_handle);

        /**
         * @brief Launches battery_monitor_task.
         *
         * This function is used to get around the fact xTaskCreate() from the freertos api requires a static task function.
         *
         * To prevent having to write the battery_monitor_task from the context of a static function, this launches the battery_monitor_task
         * from the BatteryMonitor object passed into xTaskCreate().
         *
         * @param arg a void pointer to the BatteryMonitor object from xTaskCreate() call
         * @return void, nothing to return
         */
        static void battery_monitor_task_trampoline(void* arg);

        /**
         * @brief Battery monitoring task.
         *
         * Task responsible for reading battery voltage level with ADC. Takes a set of samples, filters them, then averages them every 5ms.
         * The resulting voltage is passed to device frontend model.
         * @return void, nothing to return
         */
        void battery_monitor_task();

        /**
         * @brief Takes average of passed samples.
         *
         * @param buffer buffer containing ADC samples
         * @param length the length of the buffer containing the samples
         * @return void, nothing to return
         */
        int take_average(int* buffer, int length);

        /**
         * @brief Filters a sample according to alpha coefficient and length of sample window.
         *
         * @param sample The sample to filter
         * @param alpha The alpha coefficient to filter with
         * @param window_length The length of the sample window to filter.
         *
         * @return The filtered sample.
         */
        int iir_filter(int sample, float alpha, uint16_t window_length);

        /**
         * @brief Takes the sample_count amount of samples with ADC and saves them to buffer.
         *
         * @param buffer buffer to save the samples in
         * @param sample_count The amount of samples to take.
         *
         * @return void, nothing to return
         */
        void take_samples(int* buffer, int sample_count);

        /**
         * @brief Converts battery voltage to battery state of charge during discharging.
         *
         * Converts battery voltage to battery state of charge during discharging. The equation
         * was obtained via MATLAB using the discharging curve of the battery. It was modeled as a third
         * order polynomial using MATLAB's polyfit method. 
         * 
         * @param voltage The voltage (in mv) to convert to SOC percentage
         *
         * @return The calculated SOC percentage.
         */
        uint8_t discharge_voltage_to_soc(float voltage);

             /**
         * @brief Converts battery voltage to battery state of charge during charging.
         *
         * Converts battery voltage to battery state of charge during charging. The equation
         * was obtained via MATLAB using the charging curve of the battery. It was modeled as a third
         * order polynomial using MATLAB's polyfit method. 
         * 
         * @param voltage The voltage (in mv) to convert to SOC percentage
         *
         * @return The calculated SOC percentage.
         */
        uint8_t charge_voltage_to_soc(float voltage);

        Device& d;                                                        ///<reference to device fontend to update with new battery voltage levels
        TaskHandle_t battery_monitor_task_hdl;                            ///<battery monitor task handle
        adc_oneshot_unit_handle_t adc_hdl;                                ///< Adc peripheral handle for oneshot mode
        adc_cali_handle_t adc_calibrate_ch3_hdl;                          ///< adc calibration handle for channel 3
        bool channel_calibrated;                                          ///<whether or not the channel has been calibrated
        static const constexpr uint16_t SAMPLE_BUFFER_LENGTH = 200;       ///<Length of sample buffer.
        static const constexpr float VBAT_SCALE_FACTOR = (1.0f / 0.545f); /// Scaling factor according to onboard voltage divider.
        static const constexpr char* TAG = "BatteryMonitor";              ///<class tag, used in debug logs
};
