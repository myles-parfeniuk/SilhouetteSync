// standard library includes
#include <iostream>
// esp-idf includes
#include "esp_err.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
// in-house includes
#include "../defs/wireless_com_defs.hpp"
#include "PacketTransceiver.hpp"

class UDPServer
{
    public:
        UDPServer();

    private:
        int16_t sock;
        PacketTransceiver packet_stream;
        uint8_t s_retry_num;
        TaskHandle_t udp_server_task_hdl; ///<UDP server task handle
        esp_event_handler_instance_t instance_any_id;
        esp_event_handler_instance_t instance_got_ip;
        bool connected;
        bool discovered; 

        static const constexpr char* WIFI_SSID = "TELUS1544";
        static const constexpr char* WIFI_PASS = "88htrk5yf9";
        static const constexpr uint16_t PORT = 49160;
        static const constexpr uint16_t MAX_CONNECTION_ATTEMPTS = 2;
        static const constexpr int SOCKET_TIMEOUT_MS = 500; 
        static const constexpr char* TAG = "UDPServer";

        void wifi_init_sta();
        static void udp_server_task_trampoline(void* arg);
        static void event_handler_trampoline(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
        void udp_server_task();
        void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
        void handle_connection_failed();
        void handle_connection_retry();
        void handle_connection(void* event_data);
        void handle_start();
        bool open_udp_socket();
        void close_udp_socket();
        void set_socket_broadcast(bool broadcast_en);
        void set_socket_timeout(int timeout_ms);
        void parse_request(payload_t* transmit_buffer);
        void retransmit();
};
