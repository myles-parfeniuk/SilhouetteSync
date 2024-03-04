#pragma once
#include <stdint.h>

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

typedef struct payload_t
{
        uint8_t request;
        uint8_t response;
        uint8_t id;
        float quat_i;
        float quat_j;
        float quat_k;
        float quat_real;
        uint8_t accuracy;
        uint64_t time_stamp;
        payload_t()
            : request(0)
            , response(0)
            , id(0)
            , quat_i(0)
            , quat_j(0)
            , quat_k(0)
            , quat_real(0)
            , accuracy(0)
            , time_stamp(0)
        {
        }
} payload_t;

enum class Requests
{
    sample,
    tare,
    calibrate
};

enum class Responses
{
    busy = 255,
    success = 254,
    failure = 253,
    affirmative = 252,
    sampling = 0
};

static const constexpr uint8_t RETRANSMIT_DELAY_MS = 7;