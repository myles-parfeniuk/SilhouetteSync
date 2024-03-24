#pragma once
// esp-idf includes
#include "driver/gpio.h"
// in-house includes
#include "../Device.hpp"
#include "../defs/pin_definitions.hpp"
#include "../defs/pwr_management_defs.hpp"

class PowerManagement
{
    public:
        PowerManagement(Device& d);

    private:
        enum class PWROrState
        {
            USB_powered = 0,
            battery_powered = 1
        };

        Device& d;
        TaskHandle_t power_management_task_hdl;            ///<power management task handle
        EventGroupHandle_t power_management_evt_group_hdl; ///<power management event group handle

        void init_gpio();
        void set_buck_en_on_boot();
        static void power_management_task_trampoline(void* arg);
        void power_management_task();
        void set_power_state(int or_state, int charge_state);

        static void IRAM_ATTR pwr_or_state_ISR(void* arg);
        static void IRAM_ATTR charge_state_ISR(void* arg);

        static const constexpr uint8_t OR_STATE_EVT_BIT = BIT0;
        static const constexpr uint8_t CHARGE_STATE_EVT_BIT = BIT1;
        static const constexpr uint8_t ALL_POWER_MANAGEMENT_EVT_BITS = OR_STATE_EVT_BIT | CHARGE_STATE_EVT_BIT;
        static const constexpr char* TAG = "PowerManagement"; ///<class tag, used in debug logs
};