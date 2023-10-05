#pragma once
//esp-idf includes
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
//in-house includes
#include "DataControl.hpp"

enum class MenuCodes
{
    main_menu_sel,
    counter_menu_sel,
    timer_menu_sel
};

class MenuHelper
{
    public:

    static void lock_display_buffer(); 
    static void unlock_display_buffer();
    static DataControl::CallAlways<MenuCodes> active_menu; ///<navigation switch 
    private:
    static SemaphoreHandle_t display_buffer_mutex; 
};