#include "TCPServer.hpp"
#include <math.h>

TCPServer::TCPServer(Device &d):
d(d),
s_retry_num(0),
s_wifi_event_group(NULL)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) 
    {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    
    xTaskCreate(&tcp_server_task_trampoline, "tcp_server_task", 8192, this, 12, &tcp_server_task_hdl); //launch tcp server task
}

void TCPServer::event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    EventBits_t bits;

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } 
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) 
    {
        if (s_retry_num < MAX_CONNECTION_ATTEMPTS) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            s_retry_num = 0; 
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
            bits = xEventGroupWaitBits(s_wifi_event_group,
                WIFI_RETRY,
                pdTRUE,
                pdFALSE,
                portMAX_DELAY);
            esp_wifi_connect();
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } 
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) 
    {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip: " IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void TCPServer::wifi_init_sta()
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        event_handler_trampoline,
                                                        this,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        event_handler_trampoline,
                                                        this,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .threshold = {
                .authmode = WIFI_AUTH_OPEN
            },
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
        },
    };
    strcpy((char *)wifi_config.sta.ssid, WIFI_SSID);
    strcpy((char *)wifi_config.sta.password, WIFI_PASS);
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start());
}

void TCPServer::start_tcp()
{
    char addr_str[128];
    int addr_family = AF_INET;
    int ip_protocol = 0;
    int keepAlive = 1;
    int keepIdle = SOCK_KEEPALIVE_IDLE;
    int keepInterval = SOCK_KEEPALIVE_INTERVAL;
    int keepCount = SOCK_KEEPALIVE_COUNT;
    struct sockaddr_storage dest_addr;

    if (addr_family == AF_INET) 
    {
        struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
        dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
        dest_addr_ip4->sin_family = AF_INET;
        dest_addr_ip4->sin_port = htons(PORT);
        ip_protocol = IPPROTO_IP;
    }

    int listen_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
    if (listen_sock < 0) 
    {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        // vTaskDelete(NULL);
        return;
    }

    int opt = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    ESP_LOGI(TAG, "Socket created");

    int err = bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0) 
    {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        ESP_LOGE(TAG, "IPPROTO: %d", addr_family);
        goto CLEAN_UP;
    }
    ESP_LOGI(TAG, "Socket bound, port %d", PORT);

    err = listen(listen_sock, 1);
    if (err != 0) 
    {
        ESP_LOGE(TAG, "Error occurred during listen: errno %d", errno);
        goto CLEAN_UP;
    }

    while (1) 
    {

        ESP_LOGI(TAG, "Socket listening");

        struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
        socklen_t addr_len = sizeof(source_addr);
        int sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
            break;
        }

        // Set tcp keepalive option
        setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &keepIdle, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &keepInterval, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &keepCount, sizeof(int));
        // Convert ip address to string
        if (source_addr.ss_family == PF_INET) {
            inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
        }
        ESP_LOGI(TAG, "Socket accepted ip address: %s", addr_str);

       
        do_retransmit(sock);

        shutdown(sock, 0);
        close(sock);
    }

CLEAN_UP:
    close(listen_sock);
}

void TCPServer::do_retransmit(const int sock)
{
    int len;
    imu_data_t current_data; 

    payload *rx_buffer = new payload;

    do {
        len = recv(sock, rx_buffer, sizeof(payload), 0); //wait until data is sent from PC side client
        if (len < 0) ESP_LOGE(TAG, "Error occurred during receiving: errno %d", errno);
        else if (len == 0) ESP_LOGW(TAG, "Connection closed");
        else 
        {
            current_data = d.imu.data.get(); 
            rx_buffer->id = 0; 
            rx_buffer->x_heading = current_data.euler_heading.x;
            rx_buffer->y_heading = current_data.euler_heading.y;
            rx_buffer->z_heading = current_data.euler_heading.z;

            send(sock, rx_buffer, sizeof(payload), 0);
        }
        } while (len > 0);
}

void TCPServer::wait_for_ap_connection(void)
{
    bool connected = false; 
    EventBits_t bits;

    while(!connected)
    {
        //d.lan_connection_status.set(LANConnectionStatus::attempting_connection);
        /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
        * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
        bits = xEventGroupWaitBits(s_wifi_event_group,
                WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                pdTRUE,
                pdFALSE,
                portMAX_DELAY);
        /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
        * happened. */
        if (bits & WIFI_CONNECTED_BIT) 
        {
            //d.lan_connection_status.set(LANConnectionStatus::connected);
            ESP_LOGI(TAG, "connected to ap SSID:%s password:%s", WIFI_SSID, WIFI_PASS);
            connected = true; 
        } else if (bits & WIFI_FAIL_BIT) 
        {
            //d.lan_connection_status.set(LANConnectionStatus::failed_connection);
            ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s", WIFI_SSID, WIFI_PASS);
            vTaskDelay(3000/portTICK_PERIOD_MS);
            xEventGroupSetBits(s_wifi_event_group, WIFI_RETRY);
        } else 
        {
            ESP_LOGE(TAG, "UNEXPECTED EVENT");
        }

    }

    /* The event will not be processed after unregister */
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
    vEventGroupDelete(s_wifi_event_group);
}

void TCPServer:: event_handler_trampoline(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    TCPServer *local_server = (TCPServer *)arg; //cast to TCPServer pointer
    
    local_server->event_handler(arg, event_base, event_id, event_data);
}

void TCPServer::tcp_server_task_trampoline(void *arg)
{
    TCPServer *local_server = (TCPServer *)arg; //cast to TCPServer pointer

    local_server->tcp_server_task(); //launch server task
}


void TCPServer::tcp_server_task()
{
    wifi_init_sta();
    wait_for_ap_connection(); 
    while(1)
    {
      start_tcp();
    }
}