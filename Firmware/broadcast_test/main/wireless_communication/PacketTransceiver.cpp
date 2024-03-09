#include "PacketTransceiver.hpp"

PacketTransceiver::PacketTransceiver(int16_t& sock)
    :sock(sock)
{
}

bool PacketTransceiver::receive_packet(payload_t* rx_buffer)
{
    int len = -1;
    vTaskDelay(RETRANSMIT_DELAY_MS / portTICK_PERIOD_MS);
    socklen_t socklen = sizeof(source_addr);
    len = recvfrom(sock, rx_buffer, sizeof(payload_t), 0, reinterpret_cast<struct sockaddr*>(&source_addr), &socklen); // receive packet from client

    // Error occurred during receiving
    if (len < 0)
    {
        if(errno == 11)
            ESP_LOGE(TAG, "recvfrom failed, socket timed out");
        else
            ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
        return false;
    }

    return true;
}

bool PacketTransceiver::send_sample_packet(payload_t* tx_buffer)
{
    build_packet(Responses::sampling, tx_buffer);

    if (send_packet(tx_buffer))
        return true;
    else
        return false;
}

bool PacketTransceiver::send_discovered_packet(payload_t* tx_buffer)
{
    build_packet(Responses::discovered, tx_buffer);

    if (send_packet(tx_buffer))
        return true;
    else
        return false;
}

void PacketTransceiver::build_packet(Responses response, payload_t* tx_buffer)
{
    tx_buffer->response = static_cast<uint8_t>(response);
    tx_buffer->request = 0;
    tx_buffer->id = 0xDEADBEEF; 
    tx_buffer->quat_i = 0;
    tx_buffer->quat_j = 0;
    tx_buffer->quat_k = 0;
    tx_buffer->quat_real = 1.0;
    tx_buffer->accuracy = 3;
    tx_buffer->time_stamp = esp_timer_get_time();
    // ESP_LOGW(TAG, "Time: %llu", tx_buffer->time_stamp);
}

bool PacketTransceiver::send_packet(payload_t* tx_buffer)
{
    int err = -1;

    // send packet back to client
    err = sendto(sock, tx_buffer, sizeof(payload_t), 0, (struct sockaddr*) &source_addr, sizeof(source_addr));

    if (err < 0)
    {
        ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
        return false;
    }

    return true;
}