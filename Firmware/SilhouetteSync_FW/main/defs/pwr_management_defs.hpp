#pragma once

/**
 *   @brief  The different power source states of the device.
 */
enum class PowerSourceStates
{
    USB_powered_charging,      ///<USB cable connected and charging.
    USB_powered_fully_charged, ///< USB cable connected and fully charged.
    battery_powered,           ///< Running on battery power.
    max
};

/**
 *   @brief  The different power states of the device, whether it is booting, shutting down, already shut down (low power), or under normal operation.
 */
enum class PowerStates
{
    boot,             ///< Device is currently booting.
    low_power,        ///< Device has been switched off and low power mode is enabled.
    normal_operation, ///< Device is running under normal operation.
    shutdown          ///< Device is in the process of transitioning from normal operation to lowe power state.
};