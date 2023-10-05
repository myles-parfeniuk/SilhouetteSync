#include "ExampleCounterMenu.hpp"

ExampleCounterMenu::ExampleCounterMenu(Device &d):
Menu(d),
nav_switch_id(0),
press_count(0)
{
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
                MenuHelper::active_menu.set(MenuCodes::main_menu_sel);
            break;

            default:

            break; 
        }

        if(press_count > 100)
        {
            press_count = 0; 
        }

    }, true);

    d.nav_switch.pause(nav_switch_id);

}

void ExampleCounterMenu::enter()
{   
    d.nav_switch.un_pause_from_cb(nav_switch_id);
}

void ExampleCounterMenu::exit()
{
    d.nav_switch.pause_from_cb(nav_switch_id);
}

void ExampleCounterMenu::draw()
{
        static char press_count_str[10];

        sprintf(press_count_str, "%03d", press_count);
        u8g2_ClearBuffer(&d.display);
        u8g2_SetFont(&d.display, u8g2_font_logisoso16_tf);
        u8g2_DrawStr(&d.display, STR_ALIGN_CENTER("000", &d.display), 18, press_count_str);
        u8g2_SetFont(&d.display, u8g2_font_5x7_mf);
        DrawingUtils::draw_progress_bar_with_label(&d.display, 2, 22, 124, 10, press_count, 0, 100, "count progress...");
}
