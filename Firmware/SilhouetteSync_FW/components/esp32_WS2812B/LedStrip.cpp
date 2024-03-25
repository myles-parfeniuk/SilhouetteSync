#include "LedStrip.hpp"

LedStrip::LedStrip(strip_config_t strip_config)
    : led_count(strip_config.led_count)
{
    rmt_chan_hdl = NULL;
    rmt_tx_channel_config_t rgb_led_chan_cfg = {
            .gpio_num = strip_config.gpio_num,
            .clk_src = RMT_CLK_SRC_DEFAULT,
            .resolution_hz = LED_STRIP_RESOLUTION_HZ, // 10MHz
            .mem_block_symbols = 64,
            .trans_queue_depth = 4,
    };

    ESP_ERROR_CHECK(rmt_new_tx_channel(&rgb_led_chan_cfg, &rmt_chan_hdl));

    strip_encoder_hdl = NULL;
    led_strip_encoder_config_t encoder_config = {
            .resolution = LED_STRIP_RESOLUTION_HZ,
    };
    ESP_ERROR_CHECK(rmt_new_led_strip_encoder(&encoder_config, &strip_encoder_hdl));
    ESP_ERROR_CHECK(rmt_enable(rmt_chan_hdl));

    // Allocate memory for tx_buff
    tx_buffer = new uint8_t[led_count * WS2812_WORD_LENGTH];

    for (int i = 0; i < led_count; i++)
    {
        pixels.push_back({0, 0, 0});
    }
}

LedStrip::~LedStrip()
{
    delete[] tx_buffer;
}

void LedStrip::set_pixel_color(uint16_t pixel, rgb_color_t color)
{
    if (pixels.size() != 0)
    {
        pixels.at(pixel) = color;
    }
}

void LedStrip::set_strip_color(rgb_color_t color)
{
    if (pixels.size() != 0)
    {
        for (int i = 0; i < pixels.size(); i++)
        {
            pixels.at(i) = color;
        }
    }
}

std::vector<rgb_color_t>& LedStrip::get_pixel_buffer()
{
    return pixels;
}

void LedStrip::write_pixel_buffer()
{
    rmt_transmit_config_t tx_config = {
            .loop_count = 0, // no transfer loop
    };

    build_tx_buffer();

    ESP_ERROR_CHECK(rmt_transmit(rmt_chan_hdl, strip_encoder_hdl, tx_buffer, led_count * WS2812_WORD_LENGTH, &tx_config));
}

void LedStrip::clear_pixel_buffer()
{
    for (int i = 0; i < led_count; i++)
    {
        pixels.at(i) = {0, 0, 0};
    }
}

void LedStrip::build_tx_buffer()
{
    // clear tx buffer
    memset(tx_buffer, 0, led_count * WS2812_WORD_LENGTH);

    if (pixels.size() != 0)
    {
        for (int i = 0; i < led_count; i++)
        {
            tx_buffer[i * 3 + 0] = pixels.at(i).green;
            tx_buffer[i * 3 + 1] = pixels.at(i).red;
            tx_buffer[i * 3 + 2] = pixels.at(i).blue;
        }
    }
}