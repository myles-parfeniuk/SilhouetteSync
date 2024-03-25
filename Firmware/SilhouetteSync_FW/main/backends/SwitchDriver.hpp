#pragma once
// esp-idf includes
#include "esp_timer.h"
#include "esp_log.h"
#include "driver/uart.h"
// in-house includes
#include "../Device.hpp"

class SwitchDriver
{
    public:
        SwitchDriver(Device& d);

    private:
        Device& d;
        TaskHandle_t switch_task_hdl;
        void init_gpio();
        void switch_task();
        static void switch_task_trampoline(void* arg);
        bool quick_press_check();
        void released_check();
        void generate_quick_press_event();
        void generate_long_press_event();
        void generate_held_event();
        void generate_released_event();
        static void IRAM_ATTR switch_ISR(void* arg);

        static const constexpr char* TAG = "SwitchDriver"; ///<class tag, used in debug logs
        static const constexpr int64_t LONG_PRESS_EVT_TIME_US = 500000LL;
};