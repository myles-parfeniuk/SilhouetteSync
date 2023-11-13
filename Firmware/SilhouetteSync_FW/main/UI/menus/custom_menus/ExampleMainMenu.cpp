#include "ExampleMainMenu.hpp"

ExampleMainMenu::ExampleMainMenu(Device &d):
Menu(d), //call parent constructor
nav_switch_up_id(0), //initialize switch up follower id as 0
nav_switch_down_id(0), //initialize switch down follower id as 0
nav_switch_enter_id(0), //initialize switch enter follower id as 0
sel(COUNTER_MENU_SEL) //initialize current selection as counter menu
{

    /*nav switch up callback, code inside the below lambda function is executed
    whenever input is detected on the nav up input (as long as it has not been paused
    with its follower_id)*/
    nav_switch_up_id = d.nav_switch.up.event.follow([this, &d](Button::ButtonEvent up_event)
    {
        if(up_event == Button::ButtonEvent::quick_press)
        {
            sel -= 1;
            if(sel < COUNTER_MENU_SEL)
                sel = COUNTER_MENU_SEL; 
        }
        
    }, true);

    nav_switch_down_id = d.nav_switch.down.event.follow([this, &d](Button::ButtonEvent down_event)
    {
        if(down_event == Button::ButtonEvent::quick_press)
        {
            sel += 1;
            if(sel > MAX_SEL)
                sel = TIMER_MENU_SEL; 
        }

    }, true);

    nav_switch_enter_id = d.nav_switch.enter.event.follow([this, &d](Button::ButtonEvent enter_event)
    {
        if(enter_event == Button::ButtonEvent::quick_press)
        {
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
        }

    }, true);

  

    //pause the nav switch call-backs on boot
    d.nav_switch.up.event.pause(nav_switch_up_id); 
    d.nav_switch.down.event.pause(nav_switch_down_id); 
    d.nav_switch.enter.event.pause(nav_switch_enter_id); 

}

void ExampleMainMenu::enter()
{
    //unpause the nav switch call-backs on menu entry
    d.nav_switch.up.event.un_pause_from_cb(nav_switch_up_id); 
    d.nav_switch.down.event.un_pause_from_cb(nav_switch_down_id); 
    d.nav_switch.enter.event.un_pause_from_cb(nav_switch_enter_id); 
}

void ExampleMainMenu::exit()
{
    //pause the nav switch call-backs on menu exit 
    d.nav_switch.up.event.pause_from_cb(nav_switch_up_id); 
    d.nav_switch.down.event.pause_from_cb(nav_switch_down_id); 
    d.nav_switch.enter.event.pause_from_cb(nav_switch_enter_id); 
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
