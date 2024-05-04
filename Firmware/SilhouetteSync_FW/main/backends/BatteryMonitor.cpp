#include "BatteryMonitor.hpp"

BatteryMonitor::BatteryMonitor(Device& d)
    : d(d)
    , channel_calibrated(false)
{
    adc_init();
    xTaskCreate(&battery_monitor_task_trampoline, "battery_monitor_task", 6144, this, 2, &battery_monitor_task_hdl); // launch battery monitor task
}

void BatteryMonitor::adc_init()
{
    // create handle and cfg settings for ADC1

    adc_oneshot_unit_init_cfg_t adc_cfg = {
            .unit_id = ADC_UNIT_1,
    };

    // create channel cfg settings
    adc_oneshot_chan_cfg_t channel_cfg = {
            .atten = ADC_ATTEN_DB_11,
            .bitwidth = ADC_BITWIDTH_DEFAULT, // select max supported bitwidth
    };

    // declare calibration and adc handles
    adc_calibrate_ch3_hdl = NULL;
    adc_hdl = NULL;

    // setup ADC1 and attach to adc_hdl
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&adc_cfg, &adc_hdl));

    // setup channels
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_hdl, ADC_CHANNEL_3, &channel_cfg));

    // calibrate channels
    channel_calibrated = adc_calibrate_ch(ADC_UNIT_1, ADC_CHANNEL_3, ADC_ATTEN_DB_11, &adc_calibrate_ch3_hdl); // channel 3 is GPIO3
}

void BatteryMonitor::take_samples(int* buffer, int sample_count)
{
    int sample = 0;

    for (int i = 0; i < SAMPLE_BUFFER_LENGTH; i++)
    {
        adc_oneshot_read(adc_hdl, ADC_CHANNEL_3, &sample);
        buffer[i] = sample;
        buffer[i] = iir_filter(buffer[i], 0.1, SAMPLE_BUFFER_LENGTH);
        vTaskDelay(2 / portTICK_PERIOD_MS);
    }
}

bool BatteryMonitor::adc_calibrate_ch(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t* out_handle)
{
    adc_cali_handle_t handle = NULL;
    esp_err_t ret = ESP_FAIL;
    bool calibrated = false;

    if (!calibrated)
    {
        adc_cali_curve_fitting_config_t cali_cfg = {
                .unit_id = unit,
                .atten = atten,
                .bitwidth = ADC_BITWIDTH_DEFAULT,
        };

        ret = adc_cali_create_scheme_curve_fitting(&cali_cfg, &handle);

        *out_handle = handle;

        if (ret == ESP_OK)
        {
            calibrated = true;
            ESP_LOGI(TAG, "Calibration Success");
        }
        else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated)
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

int BatteryMonitor::take_average(int* buffer, int length)
{
    uint16_t i = 0;
    int32_t sum = 0;

    for (i = 0; i < length; i++)
        sum += buffer[i];

    return (int) round((float) sum / (float) length);
}

int BatteryMonitor::iir_filter(int sample, float alpha, uint16_t window_length)
{
    static float output = 0.0;
    static uint16_t count = 0;

    if (count >= window_length)
    {
        count = 0;
        output = 0.0f;
    }

    output = (1.0f - alpha) * (float) sample + alpha * output;

    count++;

    return (int) round(output);
}

void BatteryMonitor::battery_monitor_task_trampoline(void* arg)
{
    BatteryMonitor* local_battery_monitor = (BatteryMonitor*) arg;

    local_battery_monitor->battery_monitor_task();
}

uint8_t BatteryMonitor::discharge_voltage_to_soc(float voltage)
{
    int soc = round(-9.60793904232693e-07 * pow(voltage, 3) + 0.0111898302885909 * pow(voltage, 2) - 43.1825705771347 * voltage + 55263.5169294168);

    if (soc < 0)
        soc = 0;
    else if (soc > 100)
        soc = 100;

    return (uint8_t) soc;
}

uint8_t BatteryMonitor::charge_voltage_to_soc(float voltage)
{
    int soc =
            round(-5.23052000216911e-09 * pow(voltage, 3) - 1.60622939859106e-05 * pow(voltage, 2) + 0.623432964243526 * voltage - 1854.43701498477);

    if (soc < 0)
        soc = 0;
    else if (soc > 100)
        soc = 100;

    return (uint8_t) soc;
}

void BatteryMonitor::battery_monitor_task()
{
    int adc_sample_buff[SAMPLE_BUFFER_LENGTH]; // raw adc samples
    int adc_average;                           // SAMPLE_BUFFER_LENGTH adc samples averaged
    int voltage;                               // adc_average converted to mv
    float voltage_conv;                        // Actual voltage after accounting for onboard voltage divider
    uint8_t soc;

    memset(adc_sample_buff, 0, SAMPLE_BUFFER_LENGTH * sizeof(int));

    while (1)
    {

        take_samples(adc_sample_buff, SAMPLE_BUFFER_LENGTH);
        adc_average = take_average(adc_sample_buff, SAMPLE_BUFFER_LENGTH);

        if (channel_calibrated)
        {
            ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc_calibrate_ch3_hdl, adc_average, &voltage));
        }
        voltage_conv = VBAT_SCALE_FACTOR * (float) voltage;

        if (d.power_source_state.get() == PowerSourceStates::battery_powered)
        {
            // discharging case
            soc = discharge_voltage_to_soc(voltage_conv);

            if (d.battery.soc_percentage.get() > soc)
            {
                d.battery.soc_percentage.set(soc);
                NVSManager::write_uint8_value(d.battery.soc_nvs_handle, NVS_KEY_BATTERY_SOC, soc);
            }
        }
        else
        {
            // charging chase
            if (d.power_source_state.get() == PowerSourceStates::USB_powered_fully_charged)
                soc = 100;
            else
                soc = charge_voltage_to_soc(voltage_conv);

            if (d.battery.soc_percentage.get() < soc)
            {
                d.battery.soc_percentage.set(soc);
                NVSManager::write_uint8_value(d.battery.soc_nvs_handle, NVS_KEY_BATTERY_SOC, soc);
            }
        }

        d.battery.voltage.set(voltage_conv);

        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}
