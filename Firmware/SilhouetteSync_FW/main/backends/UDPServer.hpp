// esp-idf includes
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
// in-house includes
#include "../defs/pin_definitions.hpp"
#include "../Device.hpp"

class UDPServer
{
        typedef struct payload_t
        {
                uint8_t request;
                uint8_t id;
                float quat_i;
                float quat_j;
                float quat_k;
                float quat_real;
                uint8_t accuracy;
                payload_t()
                    : request(0)
                    , id(0)
                    , quat_i(0)
                    , quat_j(0)
                    , quat_k(0)
                    , quat_real(0)
                    , accuracy(0)
                {
                }
        } payload;

        enum class Requests
        {
            sample,    ///< sample request
            tare,      ///< tare request
            calibrate, ///< calibrate request
            max
        };

    public:
        UDPServer(Device& d);

    private:
        Device& d;
        uint8_t s_retry_num;
        int16_t sock;
        TaskHandle_t udp_server_task_hdl; ///<UDP server task handle
        esp_event_handler_instance_t instance_any_id;
        esp_event_handler_instance_t instance_got_ip;
        struct sockaddr_storage source_addr;
        bool connected;

        static const constexpr char* WIFI_SSID = "TELUS1544";
        static const constexpr char* WIFI_PASS = "88htrk5yf9";
        static const constexpr uint16_t PORT = 49160;
        static const constexpr uint16_t MAX_CONNECTION_ATTEMPTS = 2;
        static const constexpr uint8_t RETRANSMIT_DELAY_MS = 10;
        static const constexpr char* TAG = "UDPServer";

        void wifi_init_sta();
        static void udp_server_task_trampoline(void* arg);
        static void event_handler_trampoline(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
        void udp_server_task();
        void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
        bool open_udp_socket();
        void close_udp_socket();
        bool receive_packet(payload_t* rx_buffer);
        bool send_sample_packet(payload_t* tx_buffer);
        bool send_calibration_packet(payload_t* tx_buffer);
        void parse_request(payload_t* transmit_buffer);
        void handle_tare_request();
        void handle_calibration_request(payload_t* transmit_buffer);
        void retransmit();
};
