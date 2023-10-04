#include "display.hpp"
#include "../defs/pin_definitions.hpp"

void displayInit(u8g2_t * disp)
{
    u8g2_esp32_hal_t u8g2_esp32_hal = U8G2Hal::U8G2_ESP32_HAL_DEFAULT;
    u8g2_esp32_hal.bus.i2c.sda = pin_i2c_sda;
    u8g2_esp32_hal.bus.i2c.scl = pin_i2c_scl; 
    U8G2Hal::hal_init(u8g2_esp32_hal);
    u8g2_Setup_ssd1306_i2c_128x32_univision_f(disp, U8G2_R0, U8G2Hal::i2c_byte_cb, U8G2Hal::gpio_and_delay_cb);
    u8x8_SetI2CAddress(&(disp->u8x8),(OLED_BASE_ADDR << 1)); 

    u8g2_InitDisplay(disp);
    u8g2_SetPowerSave(disp,0);
    u8g2_ClearDisplay(disp);
    u8g2_SetFont(disp,u8g2_font_spleen6x12_me);
    u8g2_SetFontMode(disp,0);
    u8g2_SetBitmapMode(disp,0);
    u8g2_SetDrawColor(disp,1);

    u8g2_ClearBuffer(disp);
    u8g2_DrawFrame(disp, 0, 0, 128, 32);
    u8g2_SendBuffer(disp);
}
