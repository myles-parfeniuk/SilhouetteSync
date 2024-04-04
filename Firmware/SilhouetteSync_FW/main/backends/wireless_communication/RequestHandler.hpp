#pragma once
// esp-idf includes
#include "lwip/sockets.h"
// in-house includes
#include "../../defs/wireless_com_defs.hpp"
#include "../../Device.hpp"
#include "PacketTransceiver.hpp"
/**
 * @brief Class containing handler functions called as response to PC-side client requests.
 */
class RequestHandler
{
    public:
        /**
         * @brief Constructs an RequestHandler object for handling PC-side client requests.
         *
         * Creates object containing methods handle requests from PC-side client.
         *
         * @param d reference to Device frontend
         * @param packet_stream reference to PacketTransceiver object used for sending and receiving packets.
         * @return void, nothing to return
         */
        RequestHandler(Device& d, PacketTransceiver& packet_stream);

        /**
         * @brief Handles a sample request from PC-side client.
         *
         * @param transmit_buffer A pointer to a payload_t struct to load sample packet with.
         * @return void, nothing to return
         */
        void handle_sample(payload_t* transmit_buffer);

        /**
         * @brief Handles a tare request from PC-side client.
         *
         * Tares IMU and sends a sample packet as a response.
         *
         * @param transmit_buffer A pointer to a payload_t struct to load sample packet with.
         * @return void, nothing to return
         */
        void handle_tare(payload_t* transmit_buffer);

        /**
         * @brief Handles a calibration request from PC-side client.
         *
         * @param transmit_buffer A pointer to a payload_t struct to load calibration packets with.
         * @return void, nothing to return
         */
        void handle_calibration(payload_t* transmit_buffer);

    private:
        Device& d;                                                 ///<reference to Device frontend
        PacketTransceiver& packet_stream;                          ///<reference to packet stream object used in sending and receiving packets
        static const constexpr uint8_t MAX_RESPONSE_ATTEMPTS = 10; ///< Max attempts to receive an acknowledgment/affirmative response from client.
        static const constexpr char* TAG = "RequestHandler";       ///<class tag, used in debug logs
};