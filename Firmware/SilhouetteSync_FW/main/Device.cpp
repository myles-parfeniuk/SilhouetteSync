#include "Device.hpp"

Device::Device():
nav_switch(NavSwitchEvent::released),
imu{IMUState::sleep, imu_data_t(), false}
{

}