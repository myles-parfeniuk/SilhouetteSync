//standard library includes
#include <stdio.h>
#include <math.h>
//esp-idf includes
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/soc_caps.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

static constexpr uint16_t sample_buffer_length = 2000;  
const static char *TAG = "ADC DATA LOGGER";

static bool adc_calibrate_ch(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle);
static int take_average(float *sample_buff, int length);
static float iir_filter(int sample, float alpha, uint16_t window_length);
static float adc_sample_buff[sample_buffer_length];
static int adc_raw[2][10]; //adc raw reading [adc_unit][channel]
static int voltage[2][10]; //adc voltage [adc_unit][channel]

extern "C" void app_main(void)
{
    uint16_t i = 0;
    uint64_t sample_no = 0; 
    int sample = 0; 
    //create handle and cfg settings for ADC1
    adc_oneshot_unit_handle_t adc_hdl; 
    adc_oneshot_unit_init_cfg_t adc_cfg = 
    {
        .unit_id = ADC_UNIT_1,
    };

    //create channel cfg settings 
    adc_oneshot_chan_cfg_t channel_cfg =
    {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT, //select max supported bitwidth
    };

    //declare calibration status and handles
    bool ch4_cali = false;
    adc_cali_handle_t adc_cali_ch4_hdl = NULL; 

    //setup ADC1 and attach to adc_hdl
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&adc_cfg, &adc_hdl));

    //setup channels
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_hdl, ADC_CHANNEL_4 ,&channel_cfg));

    //calibrate channels
    ch4_cali = adc_calibrate_ch(ADC_UNIT_1, ADC_CHANNEL_4, ADC_ATTEN_DB_12, &adc_cali_ch4_hdl); //channel 4 is GPIO32


    

    while(1)
    {
        for(i = 0; i < sample_buffer_length; i++)
        {
            adc_oneshot_read(adc_hdl, ADC_CHANNEL_4, &sample);
            adc_sample_buff[i] = (float)sample; 
            adc_sample_buff[i] = iir_filter(adc_sample_buff[i], 0.1, sample_buffer_length);
        }

        adc_raw[0][0] = take_average(adc_sample_buff, sample_buffer_length); 

        if(ch4_cali)
        {
            ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc_cali_ch4_hdl, adc_raw[0][0], &voltage[0][0]));
            ESP_LOGI(TAG, "SampleNo.: %lld BatteryVoltage: %d mV", sample_no, voltage[0][0]);
        }

        sample_no++; 
        
        vTaskDelay(200/portTICK_PERIOD_MS);
    }

}

static bool adc_calibrate_ch(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle)
{
    adc_cali_handle_t handle = NULL;
    esp_err_t ret = ESP_FAIL;
    bool calibrated = false;

    if(!calibrated)
    {
        adc_cali_line_fitting_config_t cali_cfg = 
        {
            .unit_id = unit,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };

        ret = adc_cali_create_scheme_line_fitting(&cali_cfg, &handle);

        *out_handle = handle; 
        
        if(ret == ESP_OK)
        {
            calibrated = true; 
            ESP_LOGI(TAG, "Calibration Success");
        } 
        else if(ret == ESP_ERR_NOT_SUPPORTED || !calibrated)
        {
             ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
        }
        else
        {
             ESP_LOGE(TAG, "Invalid arg or no memory");
        }

        return calibrated; 
        
    }


    return false; 
}

static int take_average(float *sample_buff, int length)
{
    uint16_t i = 0;
    float sum = 0;

    for(i = 0; i < length; i++)
        sum += sample_buff[i];

    return (int)round(sum / (float)length); 
}

static float iir_filter(int sample, float alpha, uint16_t window_length)
{
    static float output = 0.0; 
    static uint16_t count = 0;

    if(count >= window_length)
    {
        count = 0;
        output = 0.0f; 
    }

    output = (1.0f - alpha) * (float)sample + alpha * output;

    count++; 

    return output; 
}
