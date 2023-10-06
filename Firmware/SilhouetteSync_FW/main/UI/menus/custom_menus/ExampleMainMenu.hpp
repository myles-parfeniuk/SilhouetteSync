#pragma once

//standard library includes
#include <cstring>
#include <string>
//in-house includes
#include "../Menu.hpp"
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
            COUNTER_MENU_SEL, //counter menu selected
            TIMER_MENU_SEL, //timer menu selected
            MAX_SEL //selection exceeded 
        };


        uint16_t nav_switch_id; //follower id of nav switch callback, used to pause and unpause callback on menu exit and entry

        int16_t sel; //the current menu selection, determines how button is drawn, and which menu will be entered if an enter quick-press is detected


};