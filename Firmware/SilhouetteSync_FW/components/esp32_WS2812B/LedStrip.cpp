#include "LedStrip.hpp"
#include <string.h>

LedStrip::LedStrip(strip_config_t strip_config):
led_count(strip_config.led_count)
{
    rmt_tx_channel_config_t rgb_led_chan_cfg =
    {
        .gpio_num = strip_config.gpio_num, 
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = LED_STRIP_RESOLUTION_HZ, //10MHz
        .mem_block_symbols = 64, 
        .trans_queue_depth = 4,
    };

    ESP_ERROR_CHECK(rmt_new_tx_channel(&rgb_led_chan_cfg, &strip_channel));
    create_led_strip_encoder(LED_STRIP_RESOLUTION_HZ);
    ESP_ERROR_CHECK(rmt_enable(strip_channel));

    strip_pixels = new uint8_t[WS2812_WORD_LENGTH * led_count];
    
}

LedStrip::~LedStrip()
{
    delete[] strip_pixels; 
}

void LedStrip::create_led_strip_encoder(const uint32_t resolution)
{
    uint16_t reset_ticks = LED_STRIP_RESOLUTION_HZ / 1000000 * 50 / 2; // reset code duration defaults to 50us
    strip_encoder.base.encode = encode_led_strip_cb;
    strip_encoder.base.del = del_led_strip_encoder_cb;
    strip_encoder.base.reset = reset_led_strip_encoder_cb;

        
    rmt_bytes_encoder_config_t  bytes_encoder_cfg =
    {
        .bit0 =
        {
            .duration0 = (uint16_t)(0.3 * (double)10000000UL / (double)1000000UL), //Time high = 0.3us
            .level0 = 1, 
            .duration1 = (uint16_t)(0.9 * (double)10000000UL / (double)1000000UL), //Time low = 0.9us
            .level1 = 0, 
            

        },

        .bit1 = 
        {
            .duration0 = (uint16_t)(0.9 * (double)10000000UL / (double)1000000UL), //Time high = 0.9us
            .level0 = 1, 
            .duration1 = (uint16_t)(0.3 * (double)10000000UL / (double)1000000UL), //Time low = 0.3us
            .level1 = 0, 
            
        },

        .flags=
        {
            .msb_first = 1,
        },
    };

    rmt_copy_encoder_config_t copy_encoder_config = {};

    ESP_ERROR_CHECK(rmt_new_bytes_encoder(&bytes_encoder_cfg, &strip_encoder.bytes_encoder));
    ESP_ERROR_CHECK(rmt_new_copy_encoder(&copy_encoder_config, &strip_encoder.copy_encoder));

    strip_encoder.reset_code = (rmt_symbol_word_t) {
        .duration0 = reset_ticks,
        .level0 = 0,
        .duration1 = reset_ticks,
        .level1 = 0,
    };
}

size_t LedStrip::encode_led_strip_cb(rmt_encoder_t *encoder, rmt_channel_handle_t channel, const void *primary_data, size_t data_size, rmt_encode_state_t *ret_state)
{
    rmt_led_strip_encoder_t *led_encoder = __containerof(encoder, rmt_led_strip_encoder_t, base);
    rmt_encoder_handle_t bytes_encoder = led_encoder->bytes_encoder;
    rmt_encoder_handle_t copy_encoder = led_encoder->copy_encoder;
    rmt_encode_state_t session_state = RMT_ENCODING_RESET;
    rmt_encode_state_t state = RMT_ENCODING_RESET;
    size_t encoded_symbols = 0;

    switch(led_encoder->state)
    {
        //send RGB data state
        case (int)encoder_states_t::TX_RGB:
            //recursive call
            encoded_symbols += bytes_encoder->encode(bytes_encoder, channel, primary_data, data_size, &session_state);
            
            if (session_state & RMT_ENCODING_COMPLETE)
                led_encoder->state = (int)encoder_states_t::TX_RESET; // switch to next state when current encoding session finished

            if (session_state & RMT_ENCODING_MEM_FULL) 
            {
                state = (rmt_encode_state_t)(state | RMT_ENCODING_MEM_FULL);
                goto out; // yield if there's no free space for encoding artifacts
            }

        break;

        //send reset code
        case (int)encoder_states_t::TX_RESET: 
            //recursive call
            encoded_symbols += copy_encoder->encode(copy_encoder, channel, &led_encoder->reset_code, sizeof(led_encoder->reset_code), &session_state);

            if (session_state & RMT_ENCODING_COMPLETE) 
            {
                led_encoder->state = (int)encoder_states_t::TX_RGB; // back to the initial encoding session
                state = (rmt_encode_state_t)(state | RMT_ENCODING_COMPLETE);
            }

            if (session_state & RMT_ENCODING_MEM_FULL) 
            {
                state = (rmt_encode_state_t)(state | RMT_ENCODING_MEM_FULL);
                goto out; // yield if there's no free space for encoding artifacts
            }

        break;

        default:

        break;
    }

    out:
        *ret_state = state;
        return encoded_symbols;
}

esp_err_t LedStrip::del_led_strip_encoder_cb(rmt_encoder_t *encoder)
{
    rmt_led_strip_encoder_t *led_encoder = __containerof(encoder, rmt_led_strip_encoder_t, base);
    rmt_del_encoder(led_encoder->bytes_encoder);
    rmt_del_encoder(led_encoder->copy_encoder);
    free(led_encoder);
    return ESP_OK;
}

esp_err_t LedStrip::reset_led_strip_encoder_cb(rmt_encoder_t *encoder)
{
    rmt_led_strip_encoder_t *led_encoder = __containerof(encoder, rmt_led_strip_encoder_t, base);
    rmt_encoder_reset(led_encoder->bytes_encoder);
    rmt_encoder_reset(led_encoder->copy_encoder);
    led_encoder->state = RMT_ENCODING_RESET;
    return ESP_OK;
}

void LedStrip::set_pixel_color(uint16_t pixel, rgb_color_t color)
{
    strip_pixels[WS2812_WORD_LENGTH*pixel + 0] = color.green;
    strip_pixels[WS2812_WORD_LENGTH*pixel + 1] = color.red;
    strip_pixels[WS2812_WORD_LENGTH*pixel + 2] = color.blue;
}


void LedStrip::set_strip_color(rgb_color_t color)
{   
 
    for(int i = 0; i < WS2812_WORD_LENGTH*led_count; i++)
    {
        strip_pixels[WS2812_WORD_LENGTH*i + 0] = color.green;
        strip_pixels[WS2812_WORD_LENGTH*i + 1] = color.red;
        strip_pixels[WS2812_WORD_LENGTH*i + 2] = color.blue;
    }

}

void LedStrip::write_pixel_buffer()
{
    
    rmt_transmit_config_t tx_config = 
    {
        .loop_count = 0, // no transfer loop
    };

    ESP_ERROR_CHECK(rmt_transmit(strip_channel, strip_encoder.bytes_encoder, strip_pixels, WS2812_WORD_LENGTH*led_count, &tx_config));
    ESP_ERROR_CHECK(rmt_tx_wait_all_done(strip_channel, portMAX_DELAY));
   
}

void LedStrip::clear_pixel_buffer()
{
    memset(strip_pixels, 0, WS2812_WORD_LENGTH*led_count);
}