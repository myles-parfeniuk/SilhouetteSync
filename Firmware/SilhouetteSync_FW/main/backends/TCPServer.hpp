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

#define MAX 80
#define PORT 49160
#define SA struct sockaddr

#define ESP_WIFI_SSID      "xxxxxxxxx"
#define ESP_WIFI_PASS      "xxxxxxxxx"
#define EXAMPLE_ESP_MAXIMUM_RETRY  10

#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_OPEN

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

#define KEEPALIVE_IDLE 5
#define KEEPALIVE_INTERVAL 5
#define KEEPALIVE_COUNT 3

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

static EventGroupHandle_t s_wifi_event_group;
static int s_retry_num;
static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data);

class TCPServer
{
    public:
    TCPServer(Device &d);
    TaskHandle_t tcp_server_task_hdl; ///<TCP server task handle
    private:
    Device &d; ///<reference to device fontend to grab and send any info about device (for ex. IMU samples)
    void do_retransmit(const int sock);
    void start_tcp(void);
    void wifi_init_sta(void);
    static void tcp_server_task_trampoline(void *arg); 
    void tcp_server_task(); 
};