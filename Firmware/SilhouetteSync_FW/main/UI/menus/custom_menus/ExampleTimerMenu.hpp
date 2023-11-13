#pragma once

#include "../Menu.hpp"
#include "esp_timer.h"

class ExampleTimerMenu : public Menu 
{
    public:
        ExampleTimerMenu(Device &d); 
        void enter() override;
        void exit() override;
        void draw() override; 

    private:
        uint16_t nav_switch_enter_id; //follower id of enter nav switch callback, used to pause and unpause callback on menu exit and entry
        esp_timer_handle_t timer; //timer handle
        static void timer_cb(void *arg); //timer callback function (executed every 20ms if menu is active)
        int16_t mili_seconds; //the amount of miliseconds being displayed


};