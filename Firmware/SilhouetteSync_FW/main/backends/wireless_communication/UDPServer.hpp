// standard library includes
#include <iostream>
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
#include "../../defs/wireless_com_defs.hpp"
#include "../../Device.hpp"
#include "PacketTransceiver.hpp"
#include "RequestHandler.hpp"

/**
 * @brief UDPServer backend class.
 *
 * Class responsible for communication with PC side client.
 */
class UDPServer
{
    public:
        /**
         * @brief Constructs an UDPServer backend object.
         *
         * Creates object to manage communication with PC side client and indicate LAN connection status to device frontend model.
         *
         * @param d reference to Device frontend
         * @return void, nothing to return
         */
        UDPServer(Device& d);

    private:
        Device& d; ///<reference to device fontend containing information to send to client, as well as LAN connection states to set
        PacketTransceiver packet_stream;              ///<PacketStream object used for sending/receiving to and from PC side client.
        RequestHandler request_handler;               ///<RequestHandler object, used to deal with requests sent from PC side client.
        uint8_t s_retry_num;                          ///< Current amount of LAN connection attempt retries.
        int16_t sock;                                 ///<UDP socket used for communication with PC side client
        TaskHandle_t udp_server_task_hdl;             ///<UDP server task handle
        esp_event_handler_instance_t instance_any_id; ///<Event handler instance for WI-FI events
        esp_event_handler_instance_t instance_got_ip; ///< Event handler instance for IP events
        esp_timer_handle_t discovery_timer_hdl;       ///< Timer to re-enable discovery.
        esp_timer_handle_t calibration_reset_timer_hdl;       ///< Timer to re-enable calibration.
        bool connected;                               ///< Whether or not the device is currently connected to a LAN
        bool recently_calibrated;                     ///< Whether or not the device was recently calibrated to prevent state re-entry.
        bool wifi_active;                             ///< Whether or not the wifi peripheral is booted and trying to connect to network
        bool discovered;                              ///< Whether or not the device is currently discovered by a PC side client.

        static const constexpr char* WIFI_SSID = "NETGEAR28";  ///<Wifi network name
        static const constexpr char* WIFI_PASS = "hxbc6333"; ///<Wifi network pass
        static const constexpr uint16_t PORT = 49160;          ///< Port associated with PC side client
        static const constexpr uint16_t MAX_CONNECTION_ATTEMPTS =
                2; ///< Max ammount of LAN connection attempts before displaying failure animation and retrying.
        static const constexpr int SOCKET_TIMEOUT_MS = 5000;             ///< UDP socket read timeout
        static const constexpr int MAX_CONNECTION_BACKOFF_DELAY_MS = 50; ///< Max LAN connection back off delay
        static const constexpr char* TAG = "UDPServer";                  ///<class tag, used in debug logs

        /**
         * @brief Initializes wifi peripheral.
         *
         * Initializes net interface, registers event handlers, and begins attempting to connect to LAN.
         *
         * @return void, nothing to return
         */
        void wifi_init_sta();

          /**
         * @brief Executes discovery timer callback.
         *
         * This function is used to get around the fact xTaskCreate() from the freertos api requires a static task function.
         *
         * To prevent having to write the discovery_timer_cb from the context of a static function, this launches the discovery_timer_cb
         * from the UDPServer object passed into esp_timer_create().
         *
         * @param arg a void pointer to the UDPServer object from esp_timer_create() call
         * @return void, nothing to return
         */
        static void discovery_timer_cb_trampoline(void* arg);

        
         /**
         * @brief Re-enables discovery
         * 
         * Call back to timer which is started after device is discovered, disabling responses to discovery
         * packets to prevent conflicts with other devices during the discovery process.
         * This call back re-enables responses to discovery packets.
         * 
         * @return void, nothing to return
         */
        void discovery_timer_cb();

        /**
         * @brief Launches udp_server_task.
         *
         * This function is used to get around the fact xTaskCreate() from the freertos api requires a static task function.
         *
         * To prevent having to write the udp_server_task from the context of a static function, this launches the udp_server_task
         * from the UDPServer object passed into xTaskCreate().
         *
         * @param arg a void pointer to the UDPServer object from xTaskCreate() call
         * @return void, nothing to return
         */
        static void udp_server_task_trampoline(void* arg);

        /**
         * @brief Launches event_handler
         *
         * This function is used to get around the fact esp_event_handler_instance_register() from the esp_event api requires a static task function.
         *
         * To prevent having to write the wifi event_handler from the context of a static function, this launches the event_handler()
         * from the UDPServer object passed into esp_event_handler_instance_register().
         *
         * @param arg a void pointer to the UDPServer object from esp_event_handler_instance_register() call
         * @return void, nothing to return
         */
        static void event_handler_trampoline(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

        /**
         * @brief Task responsible for handling communication with PC side client.
         *
         * This task initializes the wifi peripheral and opens a UDP socket for communication with client. It will then respond accordingly
         * to any request packets sent by said client.
         *
         * @return void, nothing to return
         */
        void udp_server_task();

        /**
         * @brief Event handler callback function for wifi events.
         *
         * Callback function that is executed when a wi-fi peripheral event occurs (for ex. connection failure, connection success, IP assigned,
         * etc...)
         *
         * @return void, nothing to return
         */
        void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

        /**
         * @brief Called by event handler after connection failure (max connection attempts exceeded)
         *
         * @return void, nothing to return
         */
        void handle_connection_failed();

        /**
         * @brief Called by event handler after connection stopped (entering low power mode)
         *
         * @return void, nothing to return
         */
        void handle_connection_stop();

        /**
         * @brief Called by event handler after connection failure (max connection attempts not exceeded)
         *
         * @return void, nothing to return
         */
        void handle_connection_retry();

        /**
         * @brief Called by event handler after successful LAN connection, when IP is assigned.
         *
         * @return void, nothing to return
         */
        void handle_connection(void* event_data);

        /**
         * @brief Called by event handler when wi-fi peripheral is initially booted.
         *
         * @return void, nothing to return
         */
        void handle_start();

        /**
         * @brief Opens a UDP socket for communication with PC side UDP client.
         *
         * @return true if successfully opened socket, false if otherwise.
         */
        bool open_udp_socket();

        /**
         * @brief Closes UDP socket used for communication with client.
         *
         * @return void, nothing to return
         */
        void close_udp_socket();

        /**
         * @brief Enable transmission of broadcast packets on UDP socket.
         *
         * @param enable True if enabling transmission of broadcast packets, false if disabling.
         *
         * @return void, nothing to return
         */
        void set_socket_broadcast(bool enable);

        /**
         * @brief Set UDP socket receive timeout.
         *
         * @param timeout_ms The socket receieve timeout in milliseconds
         * @return void, nothing to return
         */
        void set_socket_timeout(int timeout_ms);

        /**
         * @brief Parses request packet from PC side client and calls appropriate handler.
         *
         * @param transmit_buffer A pointer to the payload_t struct containing the packet to parse.
         *
         * @return void, nothing to return
         */
        void parse_request(payload_t* transmit_buffer);

        /**
         * @brief Receives request from client and calls parse_request to generate appropriate response packet.
         *
         * @return void, nothing to return
         */
        void retransmit();
};
