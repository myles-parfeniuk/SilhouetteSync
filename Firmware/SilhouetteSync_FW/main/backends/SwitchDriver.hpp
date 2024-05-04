#pragma once
// esp-idf includes
#include "esp_timer.h"
#include "esp_log.h"
#include "driver/uart.h"
// in-house includes
#include "../Device.hpp"

/**
 *
 * @brief SwitchDriver backend class.
 *
 * Class that manages SilhouetteSync's user input switch and communicate any switch events to Device frontend.
 */
class SwitchDriver
{
    public:
        /**
         * @brief Constructs an SwitchDriver backend object.
         *
         * Creates object to manage SilhouetteSync's user input switch and communicate switch events with Device frontend.
         *
         * @param d reference to Device frontend
         * @return void, nothing to return
         */
        SwitchDriver(Device& d);

    private:
        Device& d;                    ///<reference to device fontend to update with new switch events
        TaskHandle_t switch_task_hdl; ///<Switch task handle
        esp_timer_handle_t double_tap_reset_timer_hdl;
        uint8_t double_tap_counter;

        /**
         * @brief Initializes GPIO for user IO switch.
         *
         * Sets up switch GPIO as an input with a falling edge interrupt.
         *
         * @return void, nothing to return
         */
        void init_gpio();

        /**
         * @brief Task responsible for generating switch events, launched by switch ISR.
         *
         * This task is notified by ISR when any switch activity occurs, it then disables interrupts and monitors the switch,
         * generating events until it is released, upon which interrupts are re-enabled.
         *
         * @return void, nothing to return
         */
        void switch_task();

        /**
         * @brief Launches switch task.
         *
         * This function is used to get around the fact xTaskCreate() from the freertos api requires a static task function.
         *
         * To prevent having to write the switch task from the context of a static function, this launches the switch_task()
         * from the SwitchDriver object passed into xTaskCreate() call.
         *
         * @param arg a void pointer to the SwitchDriver object from xTaskCreate() call
         * @return void, nothing to return
         */
        static void switch_task_trampoline(void* arg);

        /**
         * @brief Checks for boot or shutdown state (accidental touch) on device power-up.
         *
         * @return true if boot state, false if shutdown
         */
        void handle_boot();

        /**
         * @brief Checks for a quick-press event.
         *
         * Checks for a quick press event, meaning the switch was pressed and released before LONG_PRESS_EVT_TIME_US elapsed.
         *
         * @return true if quick press event occured, false if long press event occured.
         */
        bool quick_press_check();

        /**
         * @brief Checks for switch release and generates held events.
         *
         * Generates a held event every HELD_EVT_TIME_MS until switch is released, upon which a release event is generated and the function
         * is returned from.
         *
         * @return n/a
         */
        void released_check();

        /**
         * @brief Generates quick press event.
         *
         * @return n/a
         */
        void generate_quick_press_event();

        /**
         * @brief Generates a double tap event.
         *
         * @return n/a
         */
        void generate_double_tap_event();

        /**
         * @brief Generates long press event.
         *
         * @return n/a
         */
        void generate_long_press_event();

        /**
         * @brief Generates held event.
         *
         * @return n/a
         */
        void generate_held_event();

        /**
         * @brief Generates released event.
         *
         * @return n/a
         */
        void generate_released_event();

        static void double_tap_reset_timer_cb_trampoline(void* arg);

        void double_tap_reset_timer_cb();

        /**
         * @brief Interrupt service routine registered to switch GPIO.
         *
         * Called when a falling edge is detected on switch GPIO, self disables interrupts and notifies switch task to run.
         *
         * @param arg A pointer to the SwitchDriver object passed to gpio_isr_handler_add(), used to notify switch task.
         * @return n/a
         */
        static void IRAM_ATTR switch_ISR(void* arg);

        static const constexpr int64_t LONG_PRESS_EVT_TIME_US =
                500000LL; ///<Quick press event is generated if switch is pressed and released before this time elapses, otherwise a long-press event is generated.

        static const constexpr int HELD_EVT_TIME_MS =
                50; ///< A held event is generated every HELD_EVT_TIME_MS when the switch remains held, upon releasing a release event is generated.

        static const constexpr char* TAG = "SwitchDriver"; ///<class tag, used in debug logs
};