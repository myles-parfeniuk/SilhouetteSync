#include "ExampleMainMenu.hpp"

ExampleMainMenu::ExampleMainMenu(Device &d):
Menu(d), //call parent constructor
nav_switch_id(0), //initialize switch follower id as 0
sel(COUNTER_MENU_SEL) //initialize current selection as counter menu
{
    /*nav switch callback, code inside the below lambda function is executed
    whenever input is detected on the nav switch (as long as it has not been paused
    with its follower_id)*/
    nav_switch_id = d.nav_switch.follow([this, &d](NavSwitchEvent switch_event)
    {
        switch(switch_event)
        {
            //if enter quick press is detected, enter the currently selected menu
            case NavSwitchEvent::enter_quick_press:

                switch(sel)
                {
                    case COUNTER_MENU_SEL:
                        MenuHelper::active_menu.set(MenuCodes::counter_menu_sel); 
                    break;

                    case TIMER_MENU_SEL:
                        MenuHelper::active_menu.set(MenuCodes::timer_menu_sel);
                    break;

                    default:

                    break; 
                }

            break;

            //if up quick press is detected move selection choice down
            case NavSwitchEvent::up_quick_press:

                sel -= 1;
                if(sel < COUNTER_MENU_SEL)
                    sel = COUNTER_MENU_SEL; 

            break;

            //if down quick press is detected move selection choice down
            case NavSwitchEvent::down_quick_press:
                sel += 1;
                if(sel > MAX_SEL)
                    sel = TIMER_MENU_SEL; 

            break; 

            default:

            break; 
        }

    }, true);

    d.nav_switch.pause(nav_switch_id); //pause the nav switch call-back on boot

}

void ExampleMainMenu::enter()
{
    d.nav_switch.un_pause_from_cb(nav_switch_id); //unpause navswitch call-back
}

void ExampleMainMenu::exit()
{
    d.nav_switch.pause_from_cb(nav_switch_id); //pause nav switch callback
}

void ExampleMainMenu::draw()
{
        char button_flags[2] = {U8G2_BTN_BW1, U8G2_BTN_BW1}; //button style flags (determines how button looks when it is drawn)

        //set button flags to depict which button is selected
        switch(sel)
        {
            case COUNTER_MENU_SEL:
                button_flags[COUNTER_MENU_SEL] = U8G2_BTN_BW1 |U8G2_BTN_SHADOW1;
            break;

            case TIMER_MENU_SEL:
                button_flags[TIMER_MENU_SEL] = U8G2_BTN_BW1 |U8G2_BTN_SHADOW1;
            break;

            default:

            break; 
        }


        u8g2_ClearBuffer(&d.display); //clear display buffer
        u8g2_SetFont(&d.display, u8g2_font_5x7_tf); //set the current font
        u8g2_DrawButtonUTF8(&d.display, 5, 10, button_flags[COUNTER_MENU_SEL], 70, 2, 2, "Counter Menu"); //create first button
        u8g2_DrawButtonUTF8(&d.display, 5, 26, button_flags[TIMER_MENU_SEL], 70, 2, 2, "Timer Menu"); //create second button 
}
