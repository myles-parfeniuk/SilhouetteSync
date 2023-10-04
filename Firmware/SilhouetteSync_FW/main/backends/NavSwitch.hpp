#pragma once

//esp-idf includes
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "esp_timer.h"

//in-house includes
#include "../Device.hpp"
#include "../defs/pin_definitions.hpp"
#include "../defs/nav_switch_defs.hpp"

class NavSwitch
{
    public:
    NavSwitch(Device &d);

    private:
    Device &d; 
    uint64_t switch_code; 
    bool debounced;
    
    bool press_check();
    void released_check(); 
    static void nav_switch_task_trampoline(void *nav_switch);
    void nav_switch_task();
    static void debounce_cb(void *arg);
    static void IRAM_ATTR nav_up_ISR(void *arg);
    static void IRAM_ATTR nav_down_ISR(void *arg);

    void generate_long_press_evt();
    void generate_quick_press_evt();
    void generate_held_evt();
    void generate_released_evt(); 


    TaskHandle_t nav_switch_task_hdl; ///<task handle of nav switch task
    esp_timer_handle_t debounce_timer_hdl; ///<debounce timer handle 

    static bool isr_service_installed; ///<true if GPIO ISR handler service has already been installed
    static const constexpr char* TAG = "NavSwitch"; ///<class tag, used in debug logs

    static const constexpr uint64_t debounce_time_us = 25000; 
    static const constexpr int64_t long_press_time_us = 200000;
    static const constexpr int64_t held_time_us = 200000; 

    static const constexpr uint64_t BOTH_PRESSED = pin_nav_up | pin_nav_down;
    static const constexpr uint64_t UP_PRESSED = pin_nav_up;
    static const constexpr uint64_t DOWN_PRESSED = pin_nav_down; 



};