#include "PacketTransceiver.hpp"

PacketTransceiver::PacketTransceiver(Device& d, int16_t& sock)
    : d(d)
    , sock(sock)
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

bool PacketTransceiver::send_busy_packet(payload_t* tx_buffer)
{

    build_packet(Responses::busy, tx_buffer);

    if (send_packet(tx_buffer))
        return true;
    else
        return false;
}

bool PacketTransceiver::send_failure_packet(payload_t* tx_buffer)
{

    build_packet(Responses::failure, tx_buffer);

    if (send_packet(tx_buffer))
        return true;
    else
        return false;
}

bool PacketTransceiver::send_success_packet(payload_t* tx_buffer)
{

    build_packet(Responses::success, tx_buffer);

    if (send_packet(tx_buffer))
        return true;
    else
        return false;
}

void PacketTransceiver::build_packet(Responses response, payload_t* tx_buffer)
{
    current_data = d.imu.data.get();

    tx_buffer->response = static_cast<uint8_t>(response);
    tx_buffer->request = 0;
    tx_buffer->id = d.id.get();
    tx_buffer->quat_i = current_data.quaternion_heading.i;
    tx_buffer->quat_j = current_data.quaternion_heading.j;
    tx_buffer->quat_k = current_data.quaternion_heading.k;
    tx_buffer->quat_real = current_data.quaternion_heading.real;
    tx_buffer->accuracy = current_data.quaternion_heading.accuracy;
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