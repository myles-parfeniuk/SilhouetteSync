#include "Device.hpp"

Device::Device():
imu{IMUState::sleep, imu_data_t(), false},
nav_switch{up_conf, down_conf, enter_conf}
{

}