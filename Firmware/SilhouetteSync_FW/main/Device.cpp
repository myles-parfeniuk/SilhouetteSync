#include "Device.hpp"

Device::Device():
imu{IMUState::sleep, imu_data_t(), false},
lan_connection_status(LANConnectionStatus::failed_connection)
{

}