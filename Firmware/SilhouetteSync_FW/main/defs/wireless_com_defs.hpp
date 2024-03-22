#pragma once
#include <stdint.h>
#include <cstring>

/**
 *   @brief  The different LAN connection states device can be in.
 */
enum class LANConnectionStatus
{
    attempting_connection, ///<attempting connection to LAN
    connected,             ///<currently connected to LAN
    failed_connection,     ///<failed connection to LAN
    max
};

static const constexpr uint8_t HARDWARE_ID_SZ = 32;

typedef struct payload_t
{
        uint8_t request;
        uint8_t response;
        char id[HARDWARE_ID_SZ];
        float quat_i;
        float quat_j;
        float quat_k;
        float quat_real;
        uint8_t accuracy;
        uint64_t time_stamp;
        uint8_t retransmit_delay;
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
        {
            memset(id, 0, sizeof(id));
        }
} payload_t;

enum class Requests
{
    client_discovery,
    client_sample,
    client_tare,
    client_calibrate,
};

enum class Responses
{
    server_discovered,
    server_sampling,
    server_failure,
    server_success,
    server_busy,
    client_affirmative,
    no_resp
};