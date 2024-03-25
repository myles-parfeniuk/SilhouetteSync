#pragma once

enum class PowerSourceStates
{
    USB_powered_charging,
    USB_powered_fully_charged,
    battery_powered,
    max
};

enum class PowerStates
{
    boot,
    low_power,
    normal_operation,
    shutdown
};