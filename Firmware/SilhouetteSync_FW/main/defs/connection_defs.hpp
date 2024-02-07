#pragma once
#include <stdint.h>

    /** 
    *   @brief  The different LAN connection states device can be in.
    */
    enum class LANConnectionStatus 
    {
        attempting_connection, ///<attempting connection to LAN
        connected, ///<currently connected to LAN
        failed_connection, ///<failed connection to LAN
        max
    };