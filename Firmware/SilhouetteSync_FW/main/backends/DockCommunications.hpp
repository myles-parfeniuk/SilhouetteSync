#pragma once
// standard library includes
#include <string>       
#include <sstream>
#include <math.h>
#include <algorithm>
// esp-idf includes
#include "driver/uart.h"
// in-house includes
#include "../Device.hpp"
#include "../defs/pin_defs.hpp"

class DockCommunications
{
    public:
        DockCommunications(Device& d);

    private:
        /**
         * @brief Receives a single character through UART from dock.
         *
         * Receives a single character from dock with an indefinite timeout.
         *
         * @return The character read.
         */
        char rx_char();

        /**
         * @brief Receives characters in a loop until end of line key is detected.
         *
         * Receives a single character from dock with an infinite timeout.
         *
         * @param buffer a pointer to char array to save received string
         * @param eol the end of line delimiter to read until
         * @param max_buffer_sz maximum possible string length (size of buffer)
         * @return void, nothing to return
         */
        void rx_until_eol(char* buffer, char eol, uint32_t max_buffer_sz);

        void dock_uart_send_data_packet();

        bool check_command_valid(char* buffer);

        void execute_command(char* buffer);

        void dock_uart_configure_tx();

        /**
         * @brief Task responsible for communication with charger dock through UART.
         *
         * This task listens to a dedicated rx pin for transmissions from charger dock.
         * If it detects a wake-up command from the charger dock it configures a tx pin
         * to send back information about the device.
         *
         * @return void, nothing to return
         */
        void dock_uart_task();

        /**
         * @brief Launches dock uart task.
         *
         * This function is used to get around the fact xTaskCreate() from the freertos api requires a static task function.
         *
         * To prevent having to write the dock uart task from the context of a static function, this launches the dock_uart_task()
         * from the DockCommunications object passed into xTaskCreate() call.
         *
         * @param arg a void pointer to the DockCommunications object from xTaskCreate() call
         * @return void, nothing to return
         */
        static void dock_uart_task_trampoline(void* arg);

        Device& d;
        bool awake;                      ///<Whether the device is currently in contact with charger dock, this indicates TX pin configured.
        TaskHandle_t dock_uart_task_hdl; ///<UDP server task handle

        static const constexpr uart_port_t DOCK_UART_PORT = UART_NUM_0;
        static const constexpr int64_t DOCK_UART_BAUD_RATE = 115200;
        static const constexpr int DOCK_UART_RX_BUFFER_SZ = 500;
        static const constexpr int DOCK_UART_RX_COMMAND_MAX_SZ = 200;
        static const constexpr std::string PACKET_REQUEST_COMMAND = "PACKET REQUEST";

        static const constexpr uart_config_t DOCK_UART_CFG = {
                .baud_rate = DOCK_UART_BAUD_RATE,
                .data_bits = UART_DATA_8_BITS,
                .parity = UART_PARITY_DISABLE,
                .stop_bits = UART_STOP_BITS_1,
                .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
                .source_clk = UART_SCLK_DEFAULT,
        };

        static const constexpr char* TAG = "DockCommunications"; ///<class tag, used in debug logs
};
