#pragma once
// esp-idf includes
#include "lwip/sockets.h"
// in-house includes
#include "../../defs/wireless_com_defs.hpp"
#include "../../Device.hpp"
#include "PacketTransceiver.hpp"
class RequestHandler
{
    public:
        RequestHandler(Device& d, PacketTransceiver& packet_stream);
        void handle_sample(payload_t* transmit_buffer);
        void handle_tare(payload_t* transmit_buffer);
        void handle_calibration(payload_t* transmit_buffer);

    private:
        Device& d;
        PacketTransceiver& packet_stream;
        static const constexpr uint8_t MAX_RESPONSE_ATTEMPTS = 10;
        static const constexpr char* TAG = "RequestHandler";
};