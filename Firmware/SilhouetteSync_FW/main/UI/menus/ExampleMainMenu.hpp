#pragma once

#include "Menu.hpp"

class ExampleMainMenu : public Menu 
{
    public:
        ExampleMainMenu(Device &d); 
        void enter() override;
        void exit() override;
        void draw() override; 

    private:

        enum Selections 
        {
            COUNTER_MENU_SEL,
            TIMER_MENU_SEL
        };


        uint16_t nav_switch_id; 

        int16_t sel; 


};