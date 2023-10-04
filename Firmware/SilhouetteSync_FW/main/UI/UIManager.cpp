#include "UIManager.hpp"
#include "helpers/DrawingUtils.hpp"
#include "../defs/nav_switch_defs.hpp"

UIManager::UIManager(Device &d):
d(d)
{
    display_init(); 
    xTaskCreate(&gui_core_task_trampoline, "gui_core_task", 4096, this, 5, &gui_core_task_hdl);


    d.nav_switch.follow([this, &d](NavSwitchEvent switch_event)
    {
        static uint16_t press_count = 0; 
        static char press_count_str[10];

        switch(switch_event)
        {
            case NavSwitchEvent::up_quick_press:
                press_count++; 
            break;

            case NavSwitchEvent::down_quick_press:
                press_count--;
            break;

            case NavSwitchEvent::enter_long_press:
                press_count = 0; 
            break;

            default:

            break; 
        }

        if(press_count > 100)
        {
            press_count = 0; 
        }

        sprintf(press_count_str, "%03d", press_count);

        MenuHelper::lock_display_buffer(); 
        u8g2_ClearBuffer(&d.display);
        u8g2_SetFont(&d.display, u8g2_font_logisoso16_tf);
        u8g2_DrawStr(&d.display, STR_ALIGN_CENTER("000", &d.display), 18, press_count_str);
        u8g2_SetFont(&d.display, u8g2_font_5x7_mf);
        DrawingUtils::draw_progress_bar_with_label(&d.display, 2, 22, 124, 10, press_count, 0, 100, "count progress...");
        MenuHelper::unlock_display_buffer(); 

    });
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
    u8g2_SetFont(&d.display, u8g2_font_logisoso16_tf);
    u8g2_SetFontMode(&d.display,1);
    u8g2_SetBitmapMode(&d.display,0);
    u8g2_SetDrawColor(&d.display,1);
}

void UIManager::gui_core_task_trampoline(void *ui_manager)
{
    UIManager *active_manager = (UIManager *)ui_manager;

    active_manager->gui_core_task(); 

}

void UIManager::gui_core_task()
{
    while(1)
    {
        vTaskDelay(33/portTICK_PERIOD_MS); //approximately 30Hz refresh rate
        MenuHelper::lock_display_buffer(); 
        u8g2_SendBuffer(&d.display);
        MenuHelper::unlock_display_buffer(); 
    }
}