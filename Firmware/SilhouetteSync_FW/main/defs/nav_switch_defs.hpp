#pragma once

    /** 
    *   @brief  the different kinds of button events that can be detected
    */
    enum class NavSwitchEvent 
    {
      up_quick_press,
      down_quick_press,
      enter_quick_press,
      up_long_press,
      down_long_press,
      enter_long_press, 
      up_held, 
      down_held,
      enter_held, 
      released ///<release event, generated when the current input is released
    };