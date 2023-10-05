#pragma once
//standard library includes
#include <cstring>   // Include the cstring header for memset
//third party includes
#include "u8g2.h"
//in-house includes
#include "../../Device.hpp"
#include "../helpers/MenuHelper.hpp"
#include "../helpers/DrawingUtils.hpp"

class Menu 
{
    public:
        Menu(Device &d);
        virtual void enter() = 0;
        virtual void exit() = 0;
        virtual void draw() = 0; 

    protected:
        Device &d; 
};