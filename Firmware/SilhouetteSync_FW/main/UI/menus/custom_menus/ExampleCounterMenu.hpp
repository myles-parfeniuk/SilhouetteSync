#pragma once

#include "../Menu.hpp"

class ExampleCounterMenu : public Menu 
{
    public:
        ExampleCounterMenu(Device &d); 
        void enter() override;
        void exit() override;
        void draw() override; 

    private:
        uint16_t nav_switch_id; //follower id of nav switch callback, used to pause and unpause callback on menu exit and entry
        uint16_t press_count; //the amount of button presses detected

};