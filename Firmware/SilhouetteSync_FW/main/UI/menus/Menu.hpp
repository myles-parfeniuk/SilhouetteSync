#pragma once
//standard library includes
#include <cstring>   // Include the cstring header for memset
//third party includes
#include "u8g2.h"
//in-house includes
#include "../../Device.hpp"
#include "../helpers/MenuHelper.hpp"
#include "../helpers/DrawingUtils.hpp"

/**
*
* @brief Menu Class
*
* Abstract base menu class. See custom_menus/ for implementation. 
*/
class Menu 
{
    public:
    /**
     * @brief Construct a menu object.
     * 
     * Default and only constructor for all menu objects. Should be called from child class constructor. 
     * 
     * @param d device object, contains any info that might be needed to draw menu (ex. user input, sensor reading)
     * @return void, nothing to return
     */
        Menu(Device &d);
    
    /**
     * @brief Take action on menu entry.
     * 
     * This function is executed by UI manager when the when the menu is entered.
     * Any code that is desired to be executed on menu entry should be placed here. 
     * 
     * @return void, nothing to return
     */
        virtual void enter() = 0;
    
    /**
     * @brief Take action on menu exit.
     * 
     * This function is executed by UI manager when the when the menu is exited.
     * Any code that is desired to be executed on menu exit should be placed here. 
     * 
     * @return void, nothing to return
     */
        virtual void exit() = 0;

    /**
     * @brief Writes data to display buffer. 
     * 
     * This function is executed by UI manager when the when the menu is being
     * drawn. 
     * 
     * All u8g2 draw functions should be placed in here. 
     * 
     * @return void, nothing to return
     */
        virtual void draw() = 0; 

    protected:
        Device &d; ///<device object, contains any info that might be needed to draw menu (ex. user input, sensor reading)
};