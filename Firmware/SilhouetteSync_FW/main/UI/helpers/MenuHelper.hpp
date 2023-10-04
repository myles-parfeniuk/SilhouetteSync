#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

class MenuHelper
{
    public:
    static void lock_display_buffer(); 
    static void unlock_display_buffer(); 
    private:
    static SemaphoreHandle_t display_buffer_mutex; 
};