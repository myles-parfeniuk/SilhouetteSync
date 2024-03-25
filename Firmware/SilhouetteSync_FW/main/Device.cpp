// standard library includes
#include <string.h>
// esp-idf includes
#include "esp_mac.h"
#include "esp_efuse.h"
#include "Device.hpp"

Device::Device()
    : id(get_hardware_id())
    , imu{IMUState::sleep, imu_data_t(), false}
    , lan_connection_status(LANConnectionStatus::failed_connection)
    , battery_voltage(0.0)
    , power_source_state(PowerSourceStates::battery_powered)
    , power_state(PowerStates::low_power)
    , user_sw(SwitchEvents::released)
{
    ESP_LOGW(TAG, "HARDWARE ID: %s", id.get().erase(id.get().find_last_not_of('_') + 1).c_str());
}

std::string Device::get_hardware_id()
{
    char unique_id[HARDWARE_ID_SZ];
    memset(unique_id, '_', HARDWARE_ID_SZ);

    ESP_ERROR_CHECK(esp_efuse_read_block(EFUSE_BLK3, unique_id, HARDWARE_ID_OFFSET * 8, (HARDWARE_ID_SZ - HARDWARE_ID_OFFSET) * 8));
    unique_id[HARDWARE_ID_SZ - 1] = '\0';

    return std::string(unique_id);
}