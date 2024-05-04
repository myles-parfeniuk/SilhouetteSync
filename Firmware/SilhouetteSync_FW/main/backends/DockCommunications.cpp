#include "DockCommunications.hpp"

DockCommunications::DockCommunications(Device& d)
    : d(d)
    , awake(false)
{

    int intr_alloc_flags = 0;

    ESP_ERROR_CHECK(uart_driver_install(DOCK_UART_PORT, DOCK_UART_RX_BUFFER_SZ, 0, 0, NULL, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config(DOCK_UART_PORT, &DOCK_UART_CFG));
    ESP_ERROR_CHECK(uart_set_pin(DOCK_UART_PORT, UART_PIN_NO_CHANGE, pin_dock_rx, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    xTaskCreate(&dock_uart_task_trampoline, "dock_uart_task", 4096, this, 4, &dock_uart_task_hdl); // launch UDP server task
}

char DockCommunications::rx_char()
{
    char rx_byte;
    uart_read_bytes(DOCK_UART_PORT, &rx_byte, 1, portMAX_DELAY);

    return rx_byte;
}

void DockCommunications::rx_until_eol(char* buffer, char eol, uint32_t max_buffer_sz)
{
    char rx_byte = ' ';
    uint32_t i = 0;

    while (rx_byte != eol)
    {
        if (i < max_buffer_sz)
        {
            rx_byte = rx_char();

            if (rx_byte != eol)
                buffer[i] = rx_byte;
            else
                buffer[i] = '\0'; // add null key in place of EOL delim to signify end of string

            i++;
        }
        else
        {
            buffer[i] = '\0'; // add null key in place of EOL delim to signify end of string
            rx_byte = eol;    // force end of string, max buffer size reached
        }
    }
}

void DockCommunications::dock_uart_send_data_packet()
{
    std::stringstream ss;
    std::string tx_packet;
    uint16_t battery_voltage = (uint16_t) round(d.battery.voltage.get());
    uint16_t battery_soc = (uint8_t)d.battery.soc_percentage.get();

    ss << d.id.get() << ' ' << battery_voltage << ' ' << battery_soc << '\n';
    tx_packet = ss.str();

    tx_packet.erase(std::remove(tx_packet.begin(), tx_packet.end(), '_'), tx_packet.end());

    uart_write_bytes(DOCK_UART_PORT, tx_packet.c_str(), tx_packet.size());
}

bool DockCommunications::check_command_valid(char* buffer)
{
    std::string command(buffer);

    if (command == PACKET_REQUEST_COMMAND)
        return true;

    return false;
}

void DockCommunications::execute_command(char* buffer)
{
    std::string command(buffer);

    if (command == PACKET_REQUEST_COMMAND)
    {
        if (!awake)
            dock_uart_configure_tx();

        dock_uart_send_data_packet();
    }
}

void DockCommunications::dock_uart_configure_tx()
{
    awake = true;
    ESP_ERROR_CHECK(uart_set_pin(DOCK_UART_PORT, pin_dock_tx, pin_dock_rx, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
}

void DockCommunications::dock_uart_task()
{
    char rx_buffer[DOCK_UART_RX_COMMAND_MAX_SZ];

    while (1)
    {
        rx_until_eol(rx_buffer, '\n', DOCK_UART_RX_COMMAND_MAX_SZ);

        if (check_command_valid(rx_buffer))
            execute_command(rx_buffer);
    }
}

void DockCommunications::dock_uart_task_trampoline(void* arg)
{
    DockCommunications* local_dock_communications_driver = (DockCommunications*) arg;

    local_dock_communications_driver->dock_uart_task();
}
