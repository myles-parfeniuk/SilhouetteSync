#pragma once
// standard library includes
#include <stdio.h>
#include <math.h>
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

class BatteryMonitor
{
    public:
        BatteryMonitor(Device& d);

    private:
        void adc_init();
        bool adc_calibrate_ch(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t* out_handle);
        static void battery_monitor_task_trampoline(void* arg);
        void battery_monitor_task();
        int take_average(int* buffer, int length);
        int iir_filter(int sample, float alpha, uint16_t window_length);
        void take_samples(int* buffer, int sample_count);

        Device& d;
        TaskHandle_t battery_monitor_task_hdl; ///<battery monitor task handle
        adc_oneshot_unit_handle_t adc_hdl;
        adc_cali_handle_t adc_calibrate_ch3_hdl;
        bool channel_calibrated;
        static const constexpr uint16_t SAMPLE_BUFFER_LENGTH = 200;
        static const constexpr float VBAT_SCALE_FACTOR = (1.0f / 0.545f);
        static const constexpr char* TAG = "BatteryMonitor";
};
