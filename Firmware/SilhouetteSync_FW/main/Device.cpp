#include "Device.hpp"

Device::Device():
imu{IMUState::sleep, imu_data_t(), false},
nav_switch{nav_switch_evt_t::released, nav_switch_evt_t::released, nav_switch_evt_t::released}
{

}