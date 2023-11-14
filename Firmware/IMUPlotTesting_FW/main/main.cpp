#include <stdio.h>
#include "BNO08x.hpp"

extern "C" void app_main(void)
{
    BNO08x imu; //create IMU object with default wiring scheme

    imu.initialize();  //initialize IMU

    //enable gyro & game rotation vector
    imu.enable_game_rotation_vector(100);
    imu.enable_gyro(150);

    while(1)
    {
        //print absolute heading in degrees and angular velocity in Rad/s
        if(imu.data_available())
        {
            ESP_LOGW("VEL", "%.3f %.3f %.3f", imu.get_gyro_calibrated_velocity_X(), imu.get_gyro_calibrated_velocity_Y(), imu.get_gyro_calibrated_velocity_Z());
            ESP_LOGI("GRV", "%.3f %.3f %.3f", imu.get_roll(), imu.get_pitch(), imu.get_yaw());
        }
    }

}