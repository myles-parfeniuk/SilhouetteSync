// esp-idf includes
#include "esp_mac.h"
#include "Device.hpp"

Device::Device()
    : id(get_hardware_id())
    , imu{IMUState::sleep, imu_data_t(), false}
    , lan_connection_status(LANConnectionStatus::failed_connection)
{
    ESP_LOGI(TAG, "Hardware ID: %llu", id.get());
}

uint64_t Device::get_hardware_id()
{
    uint8_t mac_addr[8];

    esp_efuse_mac_get_default(mac_addr);

    return (uint64_t) ((mac_addr[0] << 40) | (mac_addr[1] << 32) | (mac_addr[2] << 24) | (mac_addr[3] << 16) | (mac_addr[4] << 8) |
                       (mac_addr[5] << 0));
}