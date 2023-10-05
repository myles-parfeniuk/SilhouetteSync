#pragma once

#include "Menu.hpp"
#include "esp_timer.h"

class ExampleTimerMenu : public Menu 
{
    public:
        ExampleTimerMenu(Device &d); 
        void enter() override;
        void exit() override;
        void draw() override; 

    private:
        uint16_t nav_switch_id;
        esp_timer_handle_t timer; 
        static void timer_cb(void *arg); 
        int16_t mili_seconds; 


};