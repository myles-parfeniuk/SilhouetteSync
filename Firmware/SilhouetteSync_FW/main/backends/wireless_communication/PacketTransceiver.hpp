#pragma once
// standard library includes
#include <iostream>
// esp-idf includes
#include "lwip/sockets.h"
#include "esp_timer.h"
// in-house includes
#include "../../defs/wireless_com_defs.hpp"
#include "../../Device.hpp"

/**
 * @brief Class containing methods to send and receive packets to and from PC-side client.
 */
class PacketTransceiver
{
    public:
        /**
         * @brief Constructs an PacketTransceiver object for sending and receiving packets to and from PC-side client.
         *
         * @param d reference to Device frontend
         * @param sock reference to UDP socket to perform sending/receiving operations with
         *
         * @return void, nothing to return
         */
        PacketTransceiver(Device& d, int16_t& sock);

        /**
         * @brief Receives a packet from PC-side client.
         *
         * @param rx_buffer a pointer to the payload_t struct to save the received packet
         *
         * @return true if packet successfully received, false if otherwise
         */
        bool receive_packet(payload_t* rx_buffer);

        /**
         * @brief Sends a sample packet to PC-side client.
         *
         * Sends a sample packet to the PC-side client containing the most recent IMU data from the device frontend model.
         *
         * @param tx_buffer a pointer to the payload_t struct to be loaded with packet
         *
         * @return true if packet successfully received, false if otherwise
         */
        bool send_sample_packet(payload_t* tx_buffer);

        /**
         * @brief Sends a busy packet to PC-side client.
         *
         * Sends a busy packet to the PC-side client containing the most recent IMU data from the device frontend model.
         * A busy packet indicates that the device is currently processing a previously requested operation like calibration.
         *
         * @param tx_buffer a pointer to the payload_t struct to be loaded with packet
         *
         * @return true if packet successfully received, false if otherwise
         */
        bool send_busy_packet(payload_t* tx_buffer);

        /**
         * @brief Sends a success packet to PC-side client.
         *
         * Sends a success packet to the PC-side client containing the most recent IMU data from the device frontend model.
         * A success packet indicates a requested operation like calibration succeeded.
         *
         * @param tx_buffer a pointer to the payload_t struct to be loaded with packet
         *
         * @return true if packet successfully received, false if otherwise
         */
        bool send_success_packet(payload_t* tx_buffer);

        /**
         * @brief Sends a failure packet to PC-side client.
         *
         * Sends a failure packet to the PC-side client containing the most recent IMU data from the device frontend model.
         * A failure packet indicates a requested operation like calibration failed.
         *
         * @param tx_buffer a pointer to the payload_t struct to be loaded with packet
         *
         * @return true if packet successfully received, false if otherwise
         */
        bool send_failure_packet(payload_t* tx_buffer);

        /**
         * @brief Sends a discovered packet to PC-side client.
         *
         * Sends a discovered packet to the PC-side client containing the most recent IMU data from the device frontend model.
         * A discovered packet indicates to the client a device has been discovered during broadcast discovery.
         *
         * @param tx_buffer a pointer to the payload_t struct to be loaded with packet
         *
         * @return true if packet successfully received, false if otherwise
         */
        bool send_discovered_packet(payload_t* tx_buffer);

    private:
        Device& d;                           ///<reference to Device frontend
        struct sockaddr_storage source_addr; ///<Address of PC side client, received and saved during broadcast discovery.
        int16_t& sock;                       ///<reference to UDP socket to be used in sending and receiving operations
        imu_data_t current_data;             ///<most recent IMU data grabbed from device frontend model

        /**
         * @brief Builds a packet to be sent.
         *
         * @param tx_buffer a pointer to the payload_t struct to be loaded with packet
         * @param response the response to load into the packet
         *
         * @return true if packet successfully received, false if otherwise
         */
        void build_packet(Responses response, payload_t* tx_buffer);

        /**
         * @brief Sends a packet to PC-side client.
         *
         * @param tx_buffer a pointer to the payload_t struct to be sent
         *
         * @return true if packet successfully received, false if otherwise
         */
        bool send_packet(payload_t* tx_buffer);

        static const constexpr char* TAG = "PacketTransceiver"; ///<class tag, used in debug logs
};