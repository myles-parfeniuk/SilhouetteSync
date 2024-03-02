#include "UDPServer.hpp"

UDPServer::UDPServer(Device& d)
    : d(d)
    , s_retry_num(0)
    , connected(false)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");

    xTaskCreate(&udp_server_task_trampoline, "udp_server_task", 8192, this, 12, &udp_server_task_hdl); // launch tcp server task
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

void UDPServer::retransmit()
{
    payload_t* transmit_buffer = new payload_t;

    if (receive_packet(transmit_buffer))
        parse_request(transmit_buffer);

    delete transmit_buffer;
    vTaskDelay(RETRANSMIT_DELAY_MS / portTICK_PERIOD_MS);
}

bool UDPServer::receive_packet(payload_t* rx_buffer)
{
    static int len = -1;

    // ESP_LOGI(TAG, "Waiting for data");
    socklen_t socklen = sizeof(source_addr);
    len = recvfrom(sock, rx_buffer, sizeof(payload_t), 0, (struct sockaddr*) &source_addr, &socklen);

    // ESP_LOGI(TAG, "Received %d bytes from %s:", len, addr_str);

    // Error occurred during receiving
    if (len < 0)
    {
        ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
        return false;
    }

    return true;
}

bool UDPServer::send_sample_packet(payload_t* tx_buffer)
{
    static char addr_str[128];
    static imu_data_t current_data;

    // Get the sender's ip address as string
    if (source_addr.ss_family == PF_INET)
    {
        inet_ntoa_r(((struct sockaddr_in*) &source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
    }
    else if (source_addr.ss_family == PF_INET6)
    {
        inet6_ntoa_r(((struct sockaddr_in6*) &source_addr)->sin6_addr, addr_str, sizeof(addr_str) - 1);
    }

    current_data = d.imu.data.get();
    tx_buffer->request = 0;
    tx_buffer->id = 0;
    tx_buffer->quat_i = current_data.quaternion_heading.i;
    tx_buffer->quat_j = current_data.quaternion_heading.j;
    tx_buffer->quat_k = current_data.quaternion_heading.k;
    tx_buffer->quat_real = current_data.quaternion_heading.real;
    tx_buffer->accuracy = current_data.quaternion_heading.accuracy;

    int err = sendto(sock, tx_buffer, sizeof(payload_t), 0, (struct sockaddr*) &source_addr, sizeof(source_addr));
    if (err < 0)
    {
        ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
        return false;
    }

    return true;
}

bool UDPServer::send_calibration_packet(payload_t* tx_buffer)
{
    static char addr_str[128];
    static imu_data_t current_data;

    // Get the sender's ip address as string
    if (source_addr.ss_family == PF_INET)
    {
        inet_ntoa_r(((struct sockaddr_in*) &source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
    }
    else if (source_addr.ss_family == PF_INET6)
    {
        inet6_ntoa_r(((struct sockaddr_in6*) &source_addr)->sin6_addr, addr_str, sizeof(addr_str) - 1);
    }

    current_data = d.imu.data.get();
    tx_buffer->request = 2;
    tx_buffer->id = 0;
    tx_buffer->quat_i = current_data.quaternion_heading.i;
    tx_buffer->quat_j = current_data.quaternion_heading.j;
    tx_buffer->quat_k = current_data.quaternion_heading.k;
    tx_buffer->quat_real = current_data.quaternion_heading.real;
    tx_buffer->accuracy = current_data.quaternion_heading.accuracy;

    int err = sendto(sock, tx_buffer, sizeof(payload_t), 0, (struct sockaddr*) &source_addr, sizeof(source_addr));
    if (err < 0)
    {
        ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
        return false;
    }

    return true;
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
            connected = false;
            d.lan_connection_status.set(LANConnectionStatus::failed_connection);
            vTaskDelay(2000 / portTICK_PERIOD_MS);
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
        connected = true;
        s_retry_num = MAX_CONNECTION_ATTEMPTS;
    }
}

void UDPServer::parse_request(payload_t* transmit_buffer)
{

    switch (transmit_buffer->request)
    {
    case (uint8_t) Requests::sample:
        send_sample_packet(transmit_buffer);
        break;

    case (uint8_t) Requests::tare:
        handle_tare_request();
        break;

    case (uint8_t) Requests::calibrate:
        handle_calibration_request(transmit_buffer);
        break;

    default:

        break;
    }
}

void UDPServer::handle_tare_request()
{
    d.imu.state.set(IMUState::tare);
}

void UDPServer::handle_calibration_request(payload_t* transmit_buffer)
{
    d.imu.state.set(IMUState::calibrate);

    do
    {
        if (receive_packet(transmit_buffer))
            send_calibration_packet(transmit_buffer);

        vTaskDelay(RETRANSMIT_DELAY_MS / portTICK_PERIOD_MS);

    } while (d.imu.state.get() == IMUState::calibrate);
}
