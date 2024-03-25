#include "UDPServer.hpp"

UDPServer::UDPServer(Device& d)
    : d(d)
    , packet_stream(d, sock)
    , request_handler(d, packet_stream)
    , s_retry_num(0)
    , connected(false)
    , discovered(false)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");

    xTaskCreate(&udp_server_task_trampoline, "udp_server_task", 4096, this, 6, &udp_server_task_hdl); // launch UDP server task
}

void UDPServer::wifi_init_sta()
{
    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, event_handler_trampoline, this, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, event_handler_trampoline, this, &instance_got_ip));

    wifi_config_t wifi_config = {
            .sta =
                    {
                            .threshold = {.authmode = WIFI_AUTH_OPEN},
                            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
                    },
    };
    strcpy((char*) wifi_config.sta.ssid, WIFI_SSID);
    strcpy((char*) wifi_config.sta.password, WIFI_PASS);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

bool UDPServer::open_udp_socket()
{
    char addr_str[128];
    struct sockaddr_in6 dest_addr;
    bool socket_open = false;

    struct sockaddr_in* dest_addr_ip4 = (struct sockaddr_in*) &dest_addr;
    dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr_ip4->sin_family = AF_INET;
    dest_addr_ip4->sin_port = htons(PORT);

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock < 0)
    {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
    }
    else
    {
        ESP_LOGI(TAG, "Socket created");

        set_socket_broadcast(true);
        set_socket_timeout(SOCKET_TIMEOUT_MS);

        int err = bind(sock, (struct sockaddr*) &dest_addr, sizeof(dest_addr));
        if (err < 0)
        {
            ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        }
        else
        {
            socket_open = true;
            ESP_LOGI(TAG, "Socket bound, port %d", PORT);
        }
    }

    return socket_open;
}

void UDPServer::close_udp_socket()
{
    if (sock != -1)
    {
        ESP_LOGE(TAG, "Shutting down socket and restarting...");
        shutdown(sock, 0);
        close(sock);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

void UDPServer::set_socket_broadcast(bool enable)
{
    int broadcast_en = (int) enable;
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast_en, sizeof(broadcast_en)) < 0)
        ESP_LOGE(TAG, "setsockopt(SO_BROADCAST) failed: errno %d", errno);

    discovered = !enable; // sending broadcast messages should only be possible if device is not discovered
}

void UDPServer::set_socket_timeout(int timeout_ms)
{
    struct timeval timeout;

    timeout.tv_sec = 0;
    timeout.tv_usec = (uint32_t) timeout_ms * 1000;

    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0)
        ESP_LOGE(TAG, "setsockopt(SO_RCVTIMEO) failed: errno %d", errno);
}

void UDPServer::retransmit()
{
    payload_t* transmit_buffer = new payload_t;

    if (packet_stream.receive_packet(transmit_buffer))
        parse_request(transmit_buffer);
    else
        set_socket_broadcast(true);

    delete transmit_buffer;
}

void UDPServer::udp_server_task_trampoline(void* arg)
{
    UDPServer* local_server = (UDPServer*) arg;
    local_server->udp_server_task();
}

void UDPServer::event_handler_trampoline(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    UDPServer* local_server = (UDPServer*) arg;
    local_server->event_handler(arg, event_base, event_id, event_data);
}

void UDPServer::udp_server_task()
{
    wifi_init_sta();

    while (1)
    {
        vTaskDelay(3000 / portTICK_PERIOD_MS);

        if (open_udp_socket())
        {
            while (connected)
            {
                retransmit();
            }

            close_udp_socket();
        }
    }
}

void UDPServer::event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT)
    {
        switch (event_id)
        {
        case WIFI_EVENT_STA_START:
            handle_start();
            break;

        case WIFI_EVENT_STA_DISCONNECTED:
            if (s_retry_num < MAX_CONNECTION_ATTEMPTS)
                handle_connection_retry();
            else
                handle_connection_failed();
            break;

        default:

            break;
        }
    }
    else if (event_base == IP_EVENT)
    {
        if (event_id == IP_EVENT_STA_GOT_IP)
            handle_connection(event_data);
    }
}

void UDPServer::handle_connection_failed()
{
    s_retry_num = 0;
    connected = false;
    d.lan_connection_status.set(LANConnectionStatus::failed_connection);
    ESP_LOGE(TAG, "Connection to AP failed.");
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    d.lan_connection_status.set(LANConnectionStatus::attempting_connection);
    esp_wifi_connect();
}

void UDPServer::handle_connection_retry()
{
    esp_wifi_connect();
    s_retry_num++;
    ESP_LOGW(TAG, "Retrying to connect to AP");
}

void UDPServer::handle_connection(void* event_data)
{
    ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
    d.lan_connection_status.set(LANConnectionStatus::connected);
    ESP_LOGI(TAG, "got ip: " IPSTR, IP2STR(&event->ip_info.ip));
    connected = true;
    s_retry_num = MAX_CONNECTION_ATTEMPTS;
}

void UDPServer::handle_start()
{
    d.lan_connection_status.set(LANConnectionStatus::attempting_connection);
    esp_wifi_connect();
}

void UDPServer::parse_request(payload_t* transmit_buffer)
{

    switch (static_cast<Requests>(transmit_buffer->request))
    {
    case Requests::client_discovery:
        if (!discovered)
        {
            ESP_LOGW(TAG, "Discovered.");
            packet_stream.send_discovered_packet(transmit_buffer);
            set_socket_broadcast(false);
        }
    case Requests::client_sample:
        request_handler.handle_sample(transmit_buffer);
        break;

    case Requests::client_tare:
        request_handler.handle_tare(transmit_buffer);
        break;

    case Requests::client_calibrate:
        request_handler.handle_calibration(transmit_buffer);
        break;

    default:

        break;
    }
}