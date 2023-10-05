#include "ExampleTimerMenu.hpp"

ExampleTimerMenu::ExampleTimerMenu(Device &d):
Menu(d),
nav_switch_id(0),
mili_seconds(0)
{
    esp_timer_create_args_t timer_conf;

    nav_switch_id = d.nav_switch.follow([this, &d](NavSwitchEvent switch_event)
    {
        switch(switch_event)
        {
            case NavSwitchEvent::enter_long_press:
                MenuHelper::active_menu.set(MenuCodes::main_menu_sel); 
            break; 

            default:

            break; 
        }

    }, true);

    d.nav_switch.pause(nav_switch_id);

    //configure timer
    timer_conf.arg = (void *)this;
    timer_conf.callback = &timer_cb;
    timer_conf.name = "timer_menu_timer";
    timer_conf.dispatch_method = ESP_TIMER_TASK; 
    ESP_ERROR_CHECK(esp_timer_create(&timer_conf, &timer));

}

void ExampleTimerMenu::enter()
{   
    d.nav_switch.un_pause_from_cb(nav_switch_id);
    esp_timer_start_periodic(timer, 20000U); //start timer to run periodically with period of 20ms

}

void ExampleTimerMenu::exit()
{
    d.nav_switch.pause_from_cb(nav_switch_id);
    esp_timer_stop(timer); //stop timer
    mili_seconds = 0; //reset ms count to 0
}

void ExampleTimerMenu::draw()
{
    static char mili_seconds_str[10];

    sprintf(mili_seconds_str, "%dms", mili_seconds);
    u8g2_ClearBuffer(&d.display);
    DrawingUtils::draw_progress_bar_with_label(&d.display, 2, 22, 124, 10, mili_seconds, 0, 3000, mili_seconds_str);

}

void ExampleTimerMenu::timer_cb(void *arg)
{
    static bool dec = false; 

    ExampleTimerMenu *active_timer_menu = (ExampleTimerMenu *)arg;

    if(!dec)
    {
        active_timer_menu->mili_seconds += 20; 

        if(active_timer_menu->mili_seconds >= 3000)
            dec = true; 
    }
    else
    {
        active_timer_menu->mili_seconds -= 20; 

        if(active_timer_menu->mili_seconds <= 0)
            dec = false;
    }
}

