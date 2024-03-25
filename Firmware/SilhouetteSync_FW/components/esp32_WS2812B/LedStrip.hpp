#pragma once

//standard library includes
#include <vector>
#include <iostream> 
#include <string.h>
//esp-idf includes
#include "esp_log.h"
#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/rmt_tx.h"
#include "driver/rmt_encoder.h"
#include "strip_encoder.h"


typedef struct rgb_color_t
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;

    inline bool operator!=(const rgb_color_t& rhs) const 
    {
        return (red != rhs.red) || (green != rhs.green) || (blue != rhs.blue);
    }

    inline bool operator==(const rgb_color_t& rhs) const 
    {
        return (red == rhs.red) && (green == rhs.green) && (blue == rhs.blue);
    }

} rgb_color_t;

typedef struct 
{
    uint16_t led_count; 
    gpio_num_t gpio_num;

}strip_config_t;



class LedStrip 
{
    public: 
        LedStrip(strip_config_t strip_config);
        ~LedStrip();
        void set_pixel_color(uint16_t pixel, rgb_color_t color);
        void set_strip_color(rgb_color_t color);
        void write_pixel_buffer();
        void clear_pixel_buffer();
        std::vector<rgb_color_t>& get_pixel_buffer(); 

    private: 
        void build_tx_buffer();
        std::vector<rgb_color_t> pixels;
        uint8_t *tx_buffer;
        uint16_t led_count; 

        rmt_channel_handle_t rmt_chan_hdl; 
        rmt_encoder_handle_t strip_encoder_hdl;

        static const constexpr uint32_t LED_STRIP_RESOLUTION_HZ = 10000000UL;
        static const constexpr uint8_t WS2812_WORD_LENGTH = 3; 
        static const constexpr char *TAG = "LedStrip";
};