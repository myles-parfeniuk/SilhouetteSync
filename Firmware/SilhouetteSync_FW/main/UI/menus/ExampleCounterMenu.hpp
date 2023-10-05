#pragma once
#include "Menu.hpp"

class ExampleCounterMenu : public Menu 
{
    public:
        ExampleCounterMenu(Device &d); 
        void enter() override;
        void exit() override;
        void draw() override; 

    private:
        uint16_t nav_switch_id; 
        uint16_t press_count; 

};