#pragma once

//standard library includes
#include <memory>
//esp-idf includes
#include "esp_timer.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

//in-house includes
#include "../defs/pin_definitions.hpp"
#include "../Device.hpp"
#include "U8G2Hal.hpp"
#include "helpers/MenuHelper.hpp"
#include "menus/custom_menus/ExampleCounterMenu.hpp"
#include "menus/custom_menus/ExampleMainMenu.hpp"
#include "menus/custom_menus/ExampleTimerMenu.hpp"


/**
*
* @brief UI Managment Class
*
* Class that manages and draws the current menu being displayed. 
*/
class UIManager
{
    public:
    /**
     * @brief Construct a DataWrapper object. 
     * 
     * Construct a UIManager object to manage a menu system.
     * 
     * @param d device object, contains any info that might be needed to draw menu (ex. user input, sensor reading)
     * @return void, nothing to return
     */
        UIManager(Device &d); 

    private:
        Device &d; ///< device object, contains any info that might be needed to draw menu (ex. user input, sensor reading)

     /**
     * @brief Initialize OLED display.
     * 
     * Sets up U8G2 and configures I2C peripheral for communication with OLED.
     * 
     * @return void, nothing to return
     */
        void display_init(); 

     /**
     * @brief Launches core GUI task.
     * 
     * Static function used to launch gui_core task such that non-static class members can be accessed.
     * This is a work around with freertos and timers requiring static call-back functions. 
     * 
     * @param ui_manager pointer to UI manager objected (referenced by 'this' keyword in constructor)
     * @return void, nothing to return
     */
        static void gui_core_task_trampoline(void *ui_manager);

     /**
     * @brief Core GUI task
     * 
     * Task responsible for drawing the active menu and sending it to the OLED. 
     * 
     * @return void, nothing to return
     */
        void gui_core_task();

     /**
     * @brief Set the menu to display.
     * 
     * Sets the current menu.
     * Exits the current menu, overwrites active_menu pointer, and enters new menu. 
     * 
     * @param new_menu pointer to the next menu to be displayed 
     * @return void, nothing to return
     */
        void set_active_menu(Menu *new_menu);

        TaskHandle_t gui_core_task_hdl; ///<task handle of nav switch task

        //menus (all children of class Menu)
        ExampleMainMenu main_menu; ///<example main menu
        ExampleCounterMenu counter_menu; ///<example counter menu
        ExampleTimerMenu timer_menu; ///<example of timer menu

        Menu * active_menu; ///<pointer to the current menu being displayed
        
        static const constexpr uint16_t oled_base_addr = 0x3C; ///< i2c address of OLED
        static const constexpr char* TAG = "UIManager"; ///<class tag, used in debug logs
};