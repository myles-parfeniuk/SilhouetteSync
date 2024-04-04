#pragma once
// esp-idf includes
#include "driver/gpio.h"
// in-house includes
#include "../Device.hpp"
#include "../defs/pin_defs.hpp"
#include "../defs/pwr_management_defs.hpp"

/**
 *
 * @brief PowerManager backend class.
 *
 * Class that communicates SilhouetteSync's USB connection state, and power state to device front end.
 * Also responsible for shutting down device by disabling buck converter when a long-press event is detected on user input switch.
 */
class PowerManager
{
    public:
        /**
         * @brief Constructs an PowerManager backend object.
         *
         * Creates object to manage SilhouetteSync's USB connection state, and power state and communicate any changes to device frontend.
         * Also allows for shutdown of device when long-press event is detected on user-input switch.
         *
         * @param d reference to Device frontend
         * @return void, nothing to return
         */
        PowerManager(Device& d);

    private:
        /**
         *   @brief  The different states the OR controller can be in (used to switch between battery and USB power)
         */
        enum class PWROrState
        {
            USB_powered = 0,    ///< External OR controller indicates USB power mode
            battery_powered = 1 ///< Extern OR controller indicates battery power mode.
        };

        Device& d;                                         ///<reference to device fontend to update with new switch events
        TaskHandle_t power_management_task_hdl;            ///<power management task handle
        EventGroupHandle_t power_management_evt_group_hdl; ///<power management event group handle

        /**
         * @brief Initializes GPIO for buck converter enable, usb connection state, and OR controller.
         *
         * @return void, nothing to return
         */
        void init_gpio();

        /**
         * @brief Sets buck converter accordingly on boot.
         *
         * If device detected to be connected to USB on boot, buck converter is disabled and device is placed into low power mode.
         * If device detected to be running off battery on boot, buck converter is enabled and device is placed into normal operation mode.
         * @return void, nothing to return
         */
        void set_buck_en_on_boot();

        /**
         * @brief Launches switch task.
         *
         * This function is used to get around the fact xTaskCreate() from the freertos api requires a static task function.
         *
         * To prevent having to write the power management task from the context of a static function, this launches the power_management_task()
         * from the PowerManager object passed into xTaskCreate() call.
         *
         * @param arg a void pointer to the PowerManager object from xTaskCreate() call
         * @return void, nothing to return
         */
        static void power_management_task_trampoline(void* arg);

        /**
         * @brief Task responsible for setting power state when changes in OR state or charge state are detected.
         *
         * This task is notified via an event group by either the pwr_or_state_ISR or charge_state_ISR when a falling or rising edge is detected on
         * either of the two pins. It debounces the event and sets the power state accordingly.
         *
         * @return void, nothing to return
         */
        void power_management_task();

        /**
         * @brief Sets power state of frontend device model according to OR controller and charge state GPIO pins.
         *
         * @param or_state the level of the or controller gpio pin
         * @param charge_state the level of the charge_state gpio pin
         *
         * @return void, nothing to return
         */
        void set_power_source_state(int or_state, int charge_state);

        /**
         * @brief Interrupt service routine registered to or controller GPIO.
         *
         * Called when a falling or rising edge is detected on OR controller GPIO, self disables interrupts and notifies power_management_task to run
         * through event group.
         *
         * @param arg A pointer to the PowerManager object passed to gpio_isr_handler_add(), used to notify power_management_task.
         * @return n/a
         */
        static void IRAM_ATTR pwr_or_state_ISR(void* arg);

        /**
         * @brief Interrupt service routine registered to charge state GPIO.
         *
         * Called when a falling or rising edge is detected on charge state GPIO, self disables interrupts and notifies power_management_task to run
         * through event group.
         *
         * @param arg A pointer to the PowerManager object passed to gpio_isr_handler_add(), used to notify power_management_task.
         * @return n/a
         */
        static void IRAM_ATTR charge_state_ISR(void* arg);

        static const constexpr uint8_t OR_STATE_EVT_BIT =
                BIT0; ///< Indicates to power_management_task that OR state ISR was responsible for posting it
        static const constexpr uint8_t CHARGE_STATE_EVT_BIT =
                BIT1; ///< Indicates to power_management_task that charge state ISR was responsible for posting it
        static const constexpr uint8_t ALL_POWER_MANAGEMENT_EVT_BITS = OR_STATE_EVT_BIT | CHARGE_STATE_EVT_BIT; ///< All event bits mask.
        static const constexpr char* TAG = "PowerManager";                                                      ///<class tag, used in debug logs
};