#pragma once

//esp-idf includes
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"

//in-house includes
#include "../Device.hpp"
#include "Button.hpp"
#include "../defs/pin_definitions.hpp"
#include "../defs/nav_switch_events.hpp"

class NavSwitch
{
    public:
    NavSwitch(Device &d);

    private:
    Device &d; 
    Button up;
    Button down; 
    NavSwitchEvent nav_event; 


    static constexpr Button::button_config_t up_conf =
    {
        .gpio_num = pin_nav_up, //gpio number connected to button, for ex.25
        .active_lo = true, //active low
        .active_hi = false, //not active high
        .pull_en = false, //internal pullup disabled
        .long_press_evt_time = 300000, //300ms long-press event generation time
        .held_evt_time = 200000, //200ms held event generation time
    };

    static constexpr Button::button_config_t down_conf =
    {
        .gpio_num = pin_nav_down, //gpio number connected to button, for ex.25
        .active_lo = true, //active low
        .active_hi = false, //not active high
        .pull_en = false, //internal pullup disabled
        .long_press_evt_time = 300000, //300ms long-press event generation time
        .held_evt_time = 200000, //200ms held event generation time
    };

    static const constexpr char* TAG = "NavSwitch"; ///<class tag, used in debug logs


};