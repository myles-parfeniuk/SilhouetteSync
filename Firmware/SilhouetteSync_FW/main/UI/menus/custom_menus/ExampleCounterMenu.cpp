#include "ExampleCounterMenu.hpp"

ExampleCounterMenu::ExampleCounterMenu(Device &d):
Menu(d), //call parent constructor
nav_switch_id(0), //initialize switch follower id as 0
press_count(0) //initialize press count as 0
{
    /*nav switch callback, code inside the below lambda function is executed
    whenever input is detected on the nav switch (as long as it has not been paused
    with its follower_id)*/
    nav_switch_id = d.nav_switch.follow([this, &d](NavSwitchEvent switch_event)
    {
        switch(switch_event)
        {
            case NavSwitchEvent::up_quick_press:
                press_count++; 
            break;

            case NavSwitchEvent::down_quick_press:
                press_count--;
            break;

            case NavSwitchEvent::enter_quick_press:
                press_count = 0; 
            break;

            case NavSwitchEvent::enter_long_press:
                MenuHelper::active_menu.set(MenuCodes::main_menu_sel); //set menu back to main if long enter press is detected 
            break;

            default:

            break; 
        }

        if(press_count > 100)
        {
            press_count = 0; 
        }

    }, true);

    d.nav_switch.pause(nav_switch_id); //pause the nav switch call-back on boot

}

void ExampleCounterMenu::enter()
{   
    d.nav_switch.un_pause_from_cb(nav_switch_id); //unpause the nav switch call-back on menu entry
}

void ExampleCounterMenu::exit()
{
    d.nav_switch.pause_from_cb(nav_switch_id); //pause the nav switch call-back on menu exit 
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
