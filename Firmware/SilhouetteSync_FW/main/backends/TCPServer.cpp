#include "TCPServer.hpp"
#include <math.h>

TCPServer::TCPServer(Device& d)
    : d(d)
    , s_retry_num(0)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");

    xTaskCreate(&tcp_server_task_trampoline, "tcp_server_task", 8192, this, 12, &tcp_server_task_hdl); // launch tcp server task
}

void TCPServer::event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    EventBits_t bits;

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        d.lan_connection_status.set(LANConnectionStatus::attempting_connection);
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (s_retry_num < MAX_CONNECTION_ATTEMPTS)
        {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        }
        else
        {
            s_retry_num = 0;
            d.lan_connection_status.set(LANConnectionStatus::failed_connection);
            vTaskDelay(3000 / portTICK_PERIOD_MS);
            d.lan_connection_status.set(LANConnectionStatus::attempting_connection);
            esp_wifi_connect();
        }
        ESP_LOGI(TAG, "connect to the AP fail");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        d.lan_connection_status.set(LANConnectionStatus::connected);
        ESP_LOGI(TAG, "got ip: " IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
    }

    /*ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));*/
}

void TCPServer::wifi_init_sta()
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

void TCPServer::start_tcp()
{
    char addr_str[128];
    int addr_family = AF_INET;
    int ip_protocol = 0;
    int keep_alive = 1;
    int keep_idle = SOCK_KEEPALIVE_IDLE;
    int keep_interval = SOCK_KEEPALIVE_INTERVAL;
    int keep_count = SOCK_KEEPALIVE_COUNT;
    int err = 0;
    bool listening = true;
    struct sockaddr_storage dest_addr;

    struct sockaddr_in* dest_addr_ip4 = (struct sockaddr_in*) &dest_addr;
    dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr_ip4->sin_family = AF_INET;
    dest_addr_ip4->sin_port = htons(PORT);

    imu_data_socket = socket(addr_family, SOCK_STREAM, IPPROTO_IP);

    if (imu_data_socket < 0)
    {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);

        return;
    }

    int opt = 1;
    setsockopt(imu_data_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    ESP_LOGI(TAG, "Socket created");

    err = bind(imu_data_socket, (struct sockaddr*) &dest_addr, sizeof(dest_addr));
    if (err != 0)
    {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        ESP_LOGE(TAG, "IPPROTO: %d", addr_family);
        // goto CLEAN_UP;
        close(imu_data_socket);
        listening = false;
    }
    else
    {
        ESP_LOGI(TAG, "Socket bound, port %d", PORT);

        err = listen(imu_data_socket, 1);
        if (err != 0)
        {
            ESP_LOGE(TAG, "Error occurred during listen: errno %d", errno);
            close(imu_data_socket);
            listening = false;
        }
    }

    while (listening)
    {

        ESP_LOGI(TAG, "Socket listening");

        struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
        socklen_t addr_len = sizeof(source_addr);
        int sock = accept(imu_data_socket, (struct sockaddr*) &source_addr, &addr_len);

        if (sock < 0)
        {
            ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
            break;
        }

        // Set tcp keepalive option
        setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &keep_alive, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &SOCK_KEEPALIVE_IDLE, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &keep_interval, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &keep_count, sizeof(int));
        // Convert ip address to string
        if (source_addr.ss_family == PF_INET)
        {
            inet_ntoa_r(((struct sockaddr_in*) &source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
        }

        ESP_LOGI(TAG, "Socket accepted ip address: %s", addr_str);

        do_retransmit(sock);

        shutdown(sock, 0);
        close(sock);
    }
}

void TCPServer::do_retransmit(const int sock)
{
    int len;
    imu_data_t current_data;

    payload_t* rx_buffer = new payload_t;

    do
    {
        len = recv(sock, rx_buffer, sizeof(payload_t), 0); // wait until data is sent from PC side client
        if (len < 0)
            ESP_LOGE(TAG, "Error occurred during receiving: errno %d", errno);
        else if (len == 0)
            ESP_LOGW(TAG, "Connection closed");
        else
        {
            current_data = d.imu.data.get();
            rx_buffer->id = 0;
            rx_buffer->x_heading = current_data.euler_heading.x;
            rx_buffer->y_heading = current_data.euler_heading.y;
            rx_buffer->z_heading = current_data.euler_heading.z;

            send(sock, rx_buffer, sizeof(payload_t), 0);
        }
    } while (len > 0);

    delete rx_buffer;
}

void TCPServer::event_handler_trampoline(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    TCPServer* local_server = (TCPServer*) arg; // cast to TCPServer pointer

    local_server->event_handler(arg, event_base, event_id, event_data);
}

void TCPServer::tcp_server_task_trampoline(void* arg)
{
    TCPServer* local_server = (TCPServer*) arg; // cast to TCPServer pointer

    local_server->tcp_server_task(); // launch server task
}

void TCPServer::tcp_server_task()
{
    wifi_init_sta();
    while (1)
    {
        start_tcp();
    }
}