#include "ExampleMainMenu.hpp"

ExampleMainMenu::ExampleMainMenu(Device &d):
Menu(d),
nav_switch_id(0),
sel(COUNTER_MENU_SEL)
{
    nav_switch_id = d.nav_switch.follow([this, &d](NavSwitchEvent switch_event)
    {
        switch(switch_event)
        {
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

            case NavSwitchEvent::up_quick_press:

                sel -= 1;
                if(sel < 0)
                    sel = COUNTER_MENU_SEL; 

            break;

            case NavSwitchEvent::down_quick_press:
                sel += 1;
                if(sel > 1)
                    sel = TIMER_MENU_SEL; 

            break; 

            default:

            break; 
        }

    }, true);

    d.nav_switch.pause(nav_switch_id);

}

void ExampleMainMenu::enter()
{
    d.nav_switch.un_pause_from_cb(nav_switch_id);
}

void ExampleMainMenu::exit()
{
    d.nav_switch.pause_from_cb(nav_switch_id);
}

void ExampleMainMenu::draw()
{
        char button_flags[2] = {U8G2_BTN_BW1, U8G2_BTN_BW1};

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


        u8g2_ClearBuffer(&d.display);
        u8g2_SetFont(&d.display, u8g2_font_5x7_tf);
        u8g2_DrawButtonUTF8(&d.display, 5, 10, button_flags[COUNTER_MENU_SEL], 70, 2, 2, "Counter Menu");
        u8g2_DrawButtonUTF8(&d.display, 5, 26, button_flags[TIMER_MENU_SEL], 70, 2, 2, "Timer Menu");
}
