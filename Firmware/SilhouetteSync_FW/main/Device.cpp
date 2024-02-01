#include "Device.hpp"

Device::Device():
imu{IMUState::sleep, imu_data_t(), false}
{

}