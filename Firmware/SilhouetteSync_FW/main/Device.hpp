#pragma once
//third party includes
#include "u8g2.h"
//in-house includes
#include "defs/imu_defs.hpp"
#include "defs/pin_definitions.hpp"
#include "DataControl.hpp"
#include "Button.hpp"


class Device
{
    public:
    
    typedef struct 
    {
        DataControl::CallAlways<IMUState> state;
        DataControl::CallAlways<imu_data_t> data;
        DataControl::CallAlways<bool> calibration_status; 
    } imu_t;

    typedef struct nav_switch_t
    {
        Button up;
        Button down;
        Button enter;
    } nav_switch_t; 


    Device(); 
    u8g2_t display; ///<oled display
    imu_t imu;
    nav_switch_t nav_switch; 


    private:
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

        static constexpr Button::button_config_t enter_conf =
        {
            .gpio_num = pin_nav_enter, //gpio number connected to button, for ex.25
            .active_lo = true, //active low
            .active_hi = false, //not active high
            .pull_en = false, //internal pullup disabled
            .long_press_evt_time = 300000, //300ms long-press event generation time
            .held_evt_time = 200000, //200ms held event generation time
        };



};