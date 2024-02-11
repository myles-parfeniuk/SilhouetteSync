#pragma once 

//esp-idf includes
#include "esp_err.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "freertos/event_groups.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

//in-house includes
#include "../defs/pin_definitions.hpp"
#include "../Device.hpp"

class TCPServer
{
    typedef struct payload_t
    {
        uint8_t id;
        float x_heading;
        float y_heading; 
        float z_heading; 
        uint8_t accuracy; 
        payload_t() :
        id(0), x_heading(0), y_heading(0), z_heading(0), accuracy(0)
        {}
    }payload;

    public:
        TCPServer(Device &d);

    private:
        Device &d; ///<reference to device fontend to grab and send any info about device (for ex. IMU samples)
        uint8_t s_retry_num;
        int16_t imu_data_socket; 
        TaskHandle_t tcp_server_task_hdl; ///<TCP server task handle
        esp_event_handler_instance_t instance_any_id;
        esp_event_handler_instance_t instance_got_ip;
        
        static const constexpr char *WIFI_SSID = "ChadNetwork";
        static const constexpr char *WIFI_PASS = "hxbc6333";
        static const constexpr uint16_t PORT = 49160; 
        static const constexpr uint16_t MAX_CONNECTION_ATTEMPTS = 2; 
        static const constexpr uint8_t SOCK_KEEPALIVE_IDLE = 5;
        static const constexpr uint8_t SOCK_KEEPALIVE_INTERVAL = 5;
        static const constexpr uint8_t SOCK_KEEPALIVE_COUNT = 3;
        static const constexpr uint8_t WIFI_CONNECTED_BIT = BIT0; 
        static const constexpr uint8_t WIFI_FAIL_BIT = BIT1; 
        static const constexpr uint8_t WIFI_RETRY = BIT2; 

        static const constexpr char *TAG = "TCPServer";

        void do_retransmit(const int sock);
        void start_tcp(void);
        void wifi_init_sta(void);
        static void tcp_server_task_trampoline(void *arg); 
        static void event_handler_trampoline(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
        void tcp_server_task(); 
        void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

};