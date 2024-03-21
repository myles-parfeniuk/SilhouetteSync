#pragma once
// standard library includes
#include <iostream>
// esp-idf includes
#include "lwip/sockets.h"
#include "esp_timer.h"
// in-house includes
#include "../../defs/wireless_com_defs.hpp"
#include "../../Device.hpp"
class PacketTransceiver
{
    public:
        PacketTransceiver(Device& d, int16_t& sock);
        bool receive_packet(payload_t* rx_buffer);
        bool send_sample_packet(payload_t* tx_buffer);
        bool send_busy_packet(payload_t* tx_buffer);
        bool send_success_packet(payload_t* tx_buffer);
        bool send_failure_packet(payload_t* tx_buffer);
        bool send_discovered_packet(payload_t* tx_buffer);

    private:
        Device& d;
        struct sockaddr_storage source_addr;
        int16_t& sock;
        imu_data_t current_data;

        void build_packet(Responses response, payload_t* tx_buffer);
        bool send_packet(payload_t* tx_buffer);

        static const constexpr char* TAG = "PacketTransceiver";
};