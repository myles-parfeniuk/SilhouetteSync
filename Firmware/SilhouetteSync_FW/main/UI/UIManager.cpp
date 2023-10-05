#include "UIManager.hpp"
#include "helpers/DrawingUtils.hpp"
#include "../defs/nav_switch_defs.hpp"

UIManager::UIManager(Device &d):
d(d),
main_menu(d),
counter_menu(d),
timer_menu(d),
active_menu(&main_menu)
{

    display_init(); 
    xTaskCreate(&gui_core_task_trampoline, "gui_core_task", 4096, this, 5, &gui_core_task_hdl); //launch gui_core_task_trampoline

    MenuHelper::active_menu.follow([this](MenuCodes next_menu){
        //set the active menu to draw whenever it is changed in the custom_menus/* code
        switch(next_menu)
        {
            case MenuCodes::main_menu_sel:
                set_active_menu(&main_menu); 
            break;

            case MenuCodes::counter_menu_sel:
                set_active_menu(&counter_menu);
            break; 

            case MenuCodes::timer_menu_sel:
                set_active_menu(&timer_menu);
            break;

            default:

            break; 
        }
    });

    MenuHelper::active_menu.set(MenuCodes::main_menu_sel); //initially set the menu to main on boot
}

void UIManager::display_init()
{

    //initialize u8g2-esp32-hal configuration struct
    u8g2_esp32_hal_t u8g2_esp32_hal = U8G2Hal::U8G2_ESP32_HAL_DEFAULT; //initialize with all default settings
    u8g2_esp32_hal.bus.i2c.sda = pin_i2c_sda; //change sda pin
    u8g2_esp32_hal.bus.i2c.scl = pin_i2c_scl; //change scl pin
    U8G2Hal::hal_init(u8g2_esp32_hal); //initialize the hal

    //call u8g2 functions to set-up display configuration and its address
    u8g2_Setup_ssd1306_i2c_128x32_univision_f(&d.display, U8G2_R0, U8G2Hal::i2c_byte_cb, U8G2Hal::gpio_and_delay_cb);
    u8x8_SetI2CAddress(&(d.display.u8x8),(oled_base_addr << 1)); 

    u8g2_InitDisplay(&d.display); //initialize display
    u8g2_SetPowerSave(&d.display,0); //take display out of power saving mode 
    u8g2_ClearDisplay(&d.display); //clear display
    u8g2_SetFont(&d.display, u8g2_font_logisoso16_tf); //set the font
    u8g2_SetFontMode(&d.display,1); //set the font mode to transparent (will XOR with drawColor)
    u8g2_SetBitmapMode(&d.display,0); //set bitmap mode to solid
    u8g2_SetDrawColor(&d.display, 1); //set draw color to solid 
}

void UIManager::gui_core_task_trampoline(void *ui_manager)
{
    UIManager *active_manager = (UIManager *)ui_manager; //cast the passed "this" pointer from xTaskCreate() into a UIManager object pointer

    active_manager->gui_core_task(); //launch the task from the casted object (this allows us to write code in a non-static function where we can access class members)

}

void UIManager::gui_core_task()
{
    while(1)
    {
        vTaskDelay(33/portTICK_PERIOD_MS); //approximately 30Hz refresh rate

        MenuHelper::lock_display_buffer(); 
        active_menu->draw();               //draw the active menu
        u8g2_SendBuffer(&d.display);       //sent the contents of the buffer to display
        MenuHelper::unlock_display_buffer(); 
    }
}

 void UIManager::set_active_menu(Menu *new_menu)
 {
        active_menu->exit(); //exit the current menu
        active_menu = new_menu; //overwrite the active menu pointer with the next menu
        active_menu->enter(); //enter the new menu
 }