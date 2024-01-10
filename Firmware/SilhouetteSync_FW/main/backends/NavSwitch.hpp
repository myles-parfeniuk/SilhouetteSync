#pragma once

//esp-idf includes
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_timer.h"

//in-house includes
#include "../Device.hpp"
#include "../defs/pin_definitions.hpp"

/**
*
* @brief NavSwitch backend class.
*
* Class that manages navigation switch, and sends any detected user input to device Device frontend.
*/
class NavSwitch
{
    public:
        /**
         * @brief Constructs an NavSwitch backend object. 
         * 
         * Creates object to manage SilhouetteSync's navigation switch and communicate any user input with Device frontend.  
         * 
         * @param d reference to Device frontend
         * @return void, nothing to return
         */
         NavSwitch(Device &d);
         TaskHandle_t nav_switch_task_hdl; ///<nav switch reading task

        /** 
        *   @brief  The three different physical inputs of a nav switch.
        */
        enum class nav_input_t 
        {
            up, 
            down, 
            enter
        };

        volatile nav_input_t detected_input; ///<Most recent nav input activity as detected on, set by nav input ISRs
         
    private:
        Device &d; ///<reference to device fontend to update with switch events

        /**
         * @brief Launches nav switch task.
         * 
         * This function is used to get around the fact xTaskCreate() from the freertos api requires a static task function.
         * 
         * To prevent having to write the nav switch task from the context of a static function, this launches the nav_switch_task()
         * from the Button object passed into xTaskCreate().
         * 
         * @param arg NavSwitch object passed as "this" pointer to xTaskCreate
         * @return void, nothing to return
         */
        static void nav_switch_task_trampoline(void *arg); 

         /**
         * @brief Task responsible for determining and generating events.
         * 
         * This task is notified by the nav input ISRs when any nav input activity is detected. 
         * It determines the correct nav switch events  for the respective input and generates them until the input is released.
         * After the input is released the task waits until it is again notified by one of the nav input ISRs
         * 
         * @return void, nothing to return
         */
        void nav_switch_task(); 

        static constexpr int64_t long_press_evt_time = 300000ULL; /**< long-press event generation time in microseconds (us), if the nav switch input is held for longer than (long_press_evt_time+25ms) 
                                                a long-press event is generated, if it is released before (long_press_evt_time+25ms) elapses a quick-press event is generated instead*/
        static constexpr int64_t held_evt_time = 200000ULL; /**< held event generation time in microseconds (us), if the nav switch input is held for longer than (25ms+long_press_evt_time+hold_evt_time),  
                                        held events will be generated every held_evt_time us*/


        /**
         * @brief Determines if a quick-press or long-press event should be set for detected nav input.
         * 
         * Generates a quick-press event if nav input is released before (25ms + long_press_evt_time) elapse.
         * Generates a long-press event otherwise. 
         * 
         * @return Returns true if nav input was released (quick-press event generated), false if nav input is still held (long-press event generated)
         */
        bool press_check();
    
        /**
         * @brief Generate held events until nav input is released.
         * 
         * Generates held events every held_evt_time microseconds.
         * If the nav input is released a release event will be generated and the function will return.
         * 
         * @return void, nothing to return
         */
        void released_check();

        /**
         * @brief Get level of detected input.
         * 
         * Get the current level of of detected nav input. The detected nav input is set by the respective GPIO ISR. 
         * 
         * @return true if nav input is currently being pressed, false if nav input inactive
         */
        bool get_nav_input_level(); 

        /**
         * @brief Generates a quick-press event and sends it to device model.
         * 
         * Generates a quick-press event by calling set() method on the Device frontend for the detected nav input. 
         * This will execute any call-backs registered to the respective input of the Device::nav_switch member. 
         * 
         * @return void, nothing to return
         */
        void generate_quick_press_evt();

        /**
         * @brief Generates a long-press event and sends it to device model.
         * 
         * Generates a long press event by calling set() method on the Device frontend for the detected nav input. 
         * This will execute any call-backs registered to the respective input of the Device::nav_switch member. 
         * 
         * @return void, nothing to return
         */
        void generate_long_press_evt();

        /**
         * @brief Generates a held event and sends it to device model.
         * 
         * Generates a held event by calling set() method on the Device frontend for the detected nav input. 
         * This will execute any call-backs registered to the respective input of the Device::nav_switch member. 
         * 
         * @return void, nothing to return
         */
        void generate_held_evt();

        /**
         * @brief Generates a released event and sends it to device model.
         * 
         * Generates a released event by calling set() method on the Device frontend for the detected nav input. 
         * This will execute any call-backs registered to the respective input of the Device::nav_switch member. 
         * 
         * @return void, nothing to return
         */
        void generate_released_evt();

        /**
         * @brief ISR handler responsible for handling activity on the up input of navigation switch.
         * 
         * Disable interrupts and notifies nav_switch_task to run. 
         * Interrupts are re-enabled upon completion of nav_switch_task. 
         * 
         * @return void, nothing to return
         */
        static void IRAM_ATTR up_handler_ISR(void *arg);

        /**
         * @brief ISR handler responsible for handling activity on the down input of navigation switch.
         * 
         * Disable interrupts and notifies nav_switch_task to run. 
         * Interrupts are re-enabled upon completion of nav_switch_task. 
         * 
         * @return void, nothing to return
         */
        static void IRAM_ATTR down_handler_ISR(void *arg);

        /**
         * @brief ISR handler responsible for handling activity on the enter input of navigation switch.
         * 
         * Disable interrupts and notifies nav_switch_task to run. 
         * Interrupts are re-enabled upon completion of nav_switch_task. 
         * 
         * @return void, nothing to return
         */
        static void IRAM_ATTR enter_handler_ISR(void *arg);
       
};