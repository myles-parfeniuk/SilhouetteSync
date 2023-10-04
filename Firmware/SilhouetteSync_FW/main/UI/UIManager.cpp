#include "UIManager.hpp"

UIManager::UIManager(Device &d):
d(d)
{
    display_init(); 
}

void UIManager::display_init()
{

    u8g2_esp32_hal_t u8g2_esp32_hal = U8G2Hal::U8G2_ESP32_HAL_DEFAULT;
    u8g2_esp32_hal.bus.i2c.sda = pin_i2c_sda;
    u8g2_esp32_hal.bus.i2c.scl = pin_i2c_scl; 
    U8G2Hal::hal_init(u8g2_esp32_hal);
    u8g2_Setup_ssd1306_i2c_128x32_univision_f(&d.display, U8G2_R0, U8G2Hal::i2c_byte_cb, U8G2Hal::gpio_and_delay_cb);
    u8x8_SetI2CAddress(&(d.display.u8x8),(oled_base_addr << 1)); 

    u8g2_InitDisplay(&d.display);
    u8g2_SetPowerSave(&d.display,0);
    u8g2_ClearDisplay(&d.display);
    u8g2_SetFont(&d.display,u8g2_font_spleen6x12_me);
    u8g2_SetFontMode(&d.display,0);
    u8g2_SetBitmapMode(&d.display,0);
    u8g2_SetDrawColor(&d.display,1);

    u8g2_ClearBuffer(&d.display);
    u8g2_DrawFrame(&d.display, 0, 0, 128, 32);
    u8g2_SendBuffer(&d.display);

}