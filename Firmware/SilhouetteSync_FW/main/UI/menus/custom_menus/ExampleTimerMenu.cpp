#include "ExampleTimerMenu.hpp"

ExampleTimerMenu::ExampleTimerMenu(Device &d):
Menu(d), //call parent constructor
nav_switch_id(0), //initialize switch follower id as 0
mili_seconds(0) //initialize miliseconds count as 0 
{
    esp_timer_create_args_t timer_conf;

    /*nav switch callback, code inside the below lambda function is executed
    whenever input is detected on the nav switch (as long as it has not been paused
    with its follower_id)*/
    nav_switch_id = d.nav_switch.follow([this, &d](NavSwitchEvent switch_event)
    {
        switch(switch_event)
        {
            case NavSwitchEvent::enter_long_press:
                MenuHelper::active_menu.set(MenuCodes::main_menu_sel); //switch to main menu if enter long press was detected
            break; 

            default:

            break; 
        }

    }, true);

    d.nav_switch.pause(nav_switch_id); //pause the nav switch call-back on boot

    //configure timer
    timer_conf.arg = (void *)this;
    timer_conf.callback = &timer_cb;
    timer_conf.name = "timer_menu_timer";
    timer_conf.dispatch_method = ESP_TIMER_TASK; 
    ESP_ERROR_CHECK(esp_timer_create(&timer_conf, &timer));

}

void ExampleTimerMenu::enter()
{   
    d.nav_switch.un_pause_from_cb(nav_switch_id); //unpause the nav switch call-back 
    esp_timer_start_periodic(timer, 20000U); //start timer to run periodically with period of 20ms

}

void ExampleTimerMenu::exit()
{
    d.nav_switch.pause_from_cb(nav_switch_id); //pause the nav switch callback
    esp_timer_stop(timer); //stop timer
    mili_seconds = 0; //reset ms count to 0
}

void ExampleTimerMenu::draw()
{
    static char mili_seconds_str[10];

    sprintf(mili_seconds_str, "%dms", mili_seconds); //print current seconds to string
    u8g2_ClearBuffer(&d.display); //clear display buffer
    u8g2_SetFont(&d.display, u8g2_font_5x7_tf); //set the current font to something that will fit inside progress bar
    DrawingUtils::draw_progress_bar_with_label(&d.display, 2, 22, 124, 10, mili_seconds, 0, 3000, mili_seconds_str); //draw timer progress bar

}

/*timer callback executed every 20ms*/
void ExampleTimerMenu::timer_cb(void *arg)
{
    static bool dec = false; //whether the milisecond count is incrementing or decrementing 

    ExampleTimerMenu *active_timer_menu = (ExampleTimerMenu *)arg; //cast 'this' pointer passed into esp_timer_create from constructor to correct object pointer

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

