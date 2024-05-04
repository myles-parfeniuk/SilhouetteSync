// standard library includes
#include <string.h>
// esp-idf includes
#include "esp_mac.h"
#include "esp_efuse.h"
#include "Device.hpp"

Device::Device()
    : id(get_hardware_id())
    , imu{IMUState::sleep, imu_data_t(), false}
    , battery{0.0, 0}
    , lan_connection_status(LANConnectionStatus::failed_connection)
    , power_source_state(PowerSourceStates::battery_powered)
    , power_state(PowerStates::boot)
    , user_sw(SwitchEvents::released)
{
    uint8_t saved_battery_soc = 0;

    ESP_LOGW(TAG, "HARDWARE ID: %s", id.get().erase(id.get().find_last_not_of('_') + 1).c_str());

    NVSManager::initialize_NVS(); 

    if(NVSManager::read_uint8_value(battery.soc_nvs_handle, NVS_KEY_BATTERY_SOC, &saved_battery_soc))
        battery.soc_percentage = saved_battery_soc;
}

std::string Device::get_hardware_id()
{
    char unique_id[HARDWARE_ID_SZ];
    memset(unique_id, '_', HARDWARE_ID_SZ);

    ESP_ERROR_CHECK(esp_efuse_read_block(EFUSE_BLK3, unique_id, HARDWARE_ID_OFFSET * 8, (HARDWARE_ID_SZ - HARDWARE_ID_OFFSET) * 8));
    unique_id[HARDWARE_ID_SZ - 1] = '\0';

    return std::string(unique_id);
}