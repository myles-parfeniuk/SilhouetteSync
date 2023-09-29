#include "NavSwitch.hpp"

NavSwitch::NavSwitch(Device &d):
d(d),
up(up_conf),
down(down_conf)
{
    up.event.follow(
        [this, &d](Button::ButtonEvent event)
        {
           
                switch(event)
                {
                    case Button::ButtonEvent::quick_press:
                        d.nav_switch.set(NavSwitchEvent::up_quick_press);
                    break;

                    case Button::ButtonEvent::long_press:
                        d.nav_switch.set(NavSwitchEvent::up_long_press);
                    break;

                    case Button::ButtonEvent::held:
                        d.nav_switch.set(NavSwitchEvent::up_held);
                    break;

                    case Button::ButtonEvent::released:
                        d.nav_switch.set(NavSwitchEvent::released);
                    break;
                }
            });

    down.event.follow(
        [this, &d](Button::ButtonEvent event)
        {
                switch(event)
                {
                    case Button::ButtonEvent::quick_press:
                        d.nav_switch.set(NavSwitchEvent::down_quick_press);
                    break;

                    case Button::ButtonEvent::long_press:
                        d.nav_switch.set(NavSwitchEvent::down_long_press);
                    break;

                    case Button::ButtonEvent::held:
                        d.nav_switch.set(NavSwitchEvent::down_held);
                    break;

                    case Button::ButtonEvent::released:
                        d.nav_switch.set(NavSwitchEvent::released);
                    break;
                }
        });

}

