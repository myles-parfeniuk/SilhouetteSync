#include "ExampleCounterMenu.hpp"

ExampleCounterMenu::ExampleCounterMenu(Device &d):
Menu(d), //call parent constructor
nav_switch_up_id(0), //initialize switch up follower id as 0
nav_switch_down_id(0), //initialize switch down follower id as 0
nav_switch_enter_id(0), //initialize switch enter follower id as 0
press_count(0) //initialize press count as 0
{
    /*nav switch up callback, code inside the below lambda function is executed
    whenever input is detected on the nav up input (as long as it has not been paused
    with its follower_id)*/
    nav_switch_up_id = d.nav_switch.up.event.follow([this, &d](Button::ButtonEvent up_event)
    {
        if(up_event == Button::ButtonEvent::quick_press)
            press_count++; //increment count on up press
        
        if(up_event == Button::ButtonEvent::held)
            press_count += 10; //increment count on up press
        
        if(press_count > 100)
            press_count = 0; 
    }, true);

    nav_switch_down_id = d.nav_switch.down.event.follow([this, &d](Button::ButtonEvent down_event)
    {
        if(down_event == Button::ButtonEvent::quick_press)
            press_count--; //decrement count on down press 
        
        if(down_event == Button::ButtonEvent::held)
            press_count -= 10; //increment count on up press

        if(press_count > 100)
            press_count = 0; 

    }, true);

    nav_switch_enter_id = d.nav_switch.enter.event.follow([this, &d](Button::ButtonEvent enter_event)
    {
        switch(enter_event)
        {
            case Button::ButtonEvent::quick_press:
                press_count = 0; //reset count on enter quickpress
            break;

            case Button::ButtonEvent::long_press:
                MenuHelper::active_menu.set(MenuCodes::main_menu_sel); //set menu back to main if long enter press is detected 
            break;

            default:
            
            break;


        }
    }, true);

    //pause the nav switch call-backs on boot
    d.nav_switch.up.event.pause(nav_switch_up_id); 
    d.nav_switch.down.event.pause(nav_switch_down_id); 
    d.nav_switch.enter.event.pause(nav_switch_enter_id); 

}

void ExampleCounterMenu::enter()
{   
    //unpause the nav switch call-backs on menu entry
    d.nav_switch.up.event.un_pause_from_cb(nav_switch_up_id); 
    d.nav_switch.down.event.un_pause_from_cb(nav_switch_down_id); 
    d.nav_switch.enter.event.un_pause_from_cb(nav_switch_enter_id); 
}

void ExampleCounterMenu::exit()
{
    //pause the nav switch call-backs on menu exit 
    d.nav_switch.up.event.pause_from_cb(nav_switch_up_id); 
    d.nav_switch.down.event.pause_from_cb(nav_switch_down_id); 
    d.nav_switch.enter.event.pause_from_cb(nav_switch_enter_id); 
}

void ExampleCounterMenu::draw()
{
        static char press_count_str[10];

        sprintf(press_count_str, "%03d", press_count); //write current press count to string
        u8g2_ClearBuffer(&d.display); //clear display buffer
        u8g2_SetFont(&d.display, u8g2_font_logisoso16_tf); //set the current font
        u8g2_DrawStr(&d.display, STR_ALIGN_CENTER("000", &d.display), 18, press_count_str); //write string horizontally aligned to center of display
        u8g2_SetFont(&d.display, u8g2_font_5x7_mf); //set the current font to something smaller for progress bar
        DrawingUtils::draw_progress_bar_with_label(&d.display, 2, 22, 124, 10, press_count, 0, 100, "count progress..."); //draw progress bar with text
}
