#pragma once

/**
 *   @brief  The different kinds of switch events that can be detected.
 */
enum class SwitchEvents
{
    quick_press, ///<quick-press event, generated if button is pressed and released quickly,
    double_tap, //<< Two quick press events occured within 50ms
    long_press,  ///<long-press event, generated if button is pressed, and not released after ~500ms
    held,        ///<held event, generated if the button is being held
    released     ///<release event, generated when button is released (note, button release event not generated for quick-press events)
};