#include "Device.hpp"

Device::Device():
nav_switch(NavSwitchEvent::released),
imu{IMUState::sleep, bno055_vector_t(), false}
{

}