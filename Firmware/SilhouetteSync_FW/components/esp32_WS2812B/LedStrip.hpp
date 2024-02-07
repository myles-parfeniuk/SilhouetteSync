#include <vector>
#include <iostream> 
#include "esp_log.h"
#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/rmt_tx.h"
#include "driver/rmt_encoder.h"

typedef struct
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;

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

    private: 
        // construct an encoder by combining primitive encoders
        typedef struct 
        {
            rmt_encoder_t base;
            rmt_encoder_t *bytes_encoder;
            rmt_encoder_t *copy_encoder;
            int state;
            rmt_symbol_word_t reset_code;
        } rmt_led_strip_encoder_t;

        uint8_t *strip_pixels;
        uint16_t led_count; 
        enum class encoder_states_t {TX_RGB, TX_RESET};
        rmt_channel_handle_t strip_channel; 
        rmt_encoder_handle_t encoder_hdl; 
        rmt_led_strip_encoder_t strip_encoder; 
        void create_led_strip_encoder(const uint32_t resolution);
        static size_t encode_led_strip_cb(rmt_encoder_t *encoder, rmt_channel_handle_t channel, const void *primary_data, size_t data_size, rmt_encode_state_t *ret_state);
        static esp_err_t del_led_strip_encoder_cb(rmt_encoder_t *encoder);
        static esp_err_t reset_led_strip_encoder_cb(rmt_encoder_t *encoder);
        static const constexpr uint32_t LED_STRIP_RESOLUTION_HZ = 10000000UL;
        static const constexpr uint8_t WS2812_WORD_LENGTH = 3; 
        static const constexpr char *TAG = "LedStrip";
};