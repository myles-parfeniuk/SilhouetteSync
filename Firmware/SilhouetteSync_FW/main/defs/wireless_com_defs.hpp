#pragma once
#include <stdint.h>
#include <cstring>
#include "pwr_management_defs.hpp"

/**
 *   @brief  The different LAN connection states device can be in.
 */
enum class LANConnectionStatus
{
    attempting_connection, ///<attempting connection to LAN
    connected,             ///<currently connected to LAN
    failed_connection,     ///<failed connection to LAN
    sleep_mode,
    max
};

static const constexpr uint8_t HARDWARE_ID_SZ = 32; ///<size of hardware ID in characters, IDs under this length are padded with underscores

typedef struct payload_t
{
        uint8_t request;         ///<Request sent by PC-side client when receiving.
        uint8_t response;        ///<Response sent by PC-side client or esp-32 server when receiving or sending, respectively.
        char id[HARDWARE_ID_SZ]; ///<String containing hardware ID.
        float quat_i;            ///< I component of quaternion.
        float quat_j;            ///< J component of quaternion.
        float quat_k;            ///< K component of quaternion.
        float quat_real;         ///< Real component of quaternion.
        uint8_t accuracy;        ///< Accuracy of quaternion.
        uint64_t time_stamp;     ///< Timestamp in microsecond ticks since esp32 boot.
        uint8_t retransmit_delay; ///< Retransmit delay sent by client, UDP server will delay retransmit_delay ms before sending a response upon receiving a packet.
        float battery_voltage;      ///< Battery voltage in millivolts.
        uint8_t soc_percentage; ///< Battery SOC (state of charge) in %
        uint8_t power_source_state; ///< Power source state (see pwr_managment_defs.hpp)
        // default packet constructor
        payload_t()
            : request(0)
            , response(0)
            , quat_i(0)
            , quat_j(0)
            , quat_k(0)
            , quat_real(0)
            , accuracy(0)
            , time_stamp(0)
            , retransmit_delay(0)
            , battery_voltage(0.0)
            , soc_percentage(0)
            , power_source_state((uint8_t) PowerSourceStates::battery_powered)
        {
            memset(id, 0, sizeof(id));
        }
} payload_t;

/**
 *   @brief  The different requests the PC-side client can send.
 */
enum class Requests
{
    client_discovery, ///<Discovery request, used in broadcast discovery process.
    client_sample,    ///<Request for a sample packet.
    client_tare,      ///<Request to tare the IMU of bracelet.
    client_calibrate, /// Request to calibrate IMU of bracelet.
};

/**
 *   @brief  The different requests the PC-side client and esp-32 server can send.
 */
enum class Responses
{
    server_discovered,  ///< Affirmative response from esp32 server to client acknowledging discovery.
    server_sampling,    ///< Response from esp32 server to client indicating it has sent a sample packet.
    server_failure,     ///< Response from esp32 server to client indicating that a requested operation (ie calibration) has failed.
    server_success,     ///< Response from esp32 server to client indicating that a requested operation (ie calibration) has succeeded.
    server_busy,        ///< Response from esp32 server to client indicating it is current busy servicing a requested operation.
    client_affirmative, ///< Affirmative response from PC-side client to esp-32 indicating it has acknowledged a success/failure response.
    no_resp             ///< No response, used in packets where a response doesn't apply, ie, the client sending a packet to initiate a sample.
};