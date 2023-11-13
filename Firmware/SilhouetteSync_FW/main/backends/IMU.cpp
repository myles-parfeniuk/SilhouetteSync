#include "IMU.hpp"

IMU::IMU(Device &d):
d(d)
{
    //follow device struct to check for current sensor mode
    d.imu.state.follow([this, &d](IMUState new_state){
        switch(new_state)
        {
            case IMUState::sleep:
                //do nothing, sample loop should exit automatically 
            break;

            case IMUState::sample:
                //if the sample task is not already running notify it to start
                if(eTaskGetState(sample_task_hdl) != eRunning) 
                    xTaskNotifyGive(sample_task_hdl);
            break;

            case IMUState::calibrate:
                imu.run_full_calibration_routine();
                d.imu.state.set(IMUState::sleep);
            break;

            default:

            break;
        }
    }, true);

    imu.initialize(); //initialize IMU unit

    //initialize imu game rotation vector and gyro
    imu.enable_game_rotation_vector(100); 
    imu.enable_gyro(150);

    xTaskCreate(&sampling_task_trampoline, "imu_sample_task", 4096, this, 5, &sample_task_hdl);
}


void IMU::sampling_task_trampoline(void *arg)
{   
    IMU * active_imu = (IMU *)arg;

    active_imu->sampling_task(); 
}

void IMU::sampling_task()
{
    imu_data_t new_data;

    ulTaskNotifyTake(pdTRUE, portMAX_DELAY); //block until notified on index 0 by state switch to sample

    while(d.imu.state.get() == IMUState::sample)
    {
 
        if(imu.data_available())
        {
            //update euler angle
            new_data.euler_heading.x = imu.get_roll(); 
            new_data.euler_heading.y = imu.get_pitch(); 
            new_data.euler_heading.z = imu.get_yaw(); 
            new_data.euler_heading.accuracy = imu.get_quat_accuracy(); 
            
            //update quaternion 
            new_data.quaternion_heading.i = imu.get_quat_I(); 
            new_data.quaternion_heading.i = imu.get_quat_J(); 
            new_data.quaternion_heading.i = imu.get_quat_K(); 
            new_data.quaternion_heading.i = imu.get_quat_real(); 
            new_data.quaternion_heading.accuracy = imu.get_quat_accuracy(); 

            //update velocity 
            new_data.velocity.x = imu.get_gyro_calibrated_velocity_X();
            new_data.velocity.y = imu.get_gyro_calibrated_velocity_Y();
            new_data.velocity.z = imu.get_gyro_calibrated_velocity_Z();
            new_data.velocity.accuracy = imu.get_gyro_accuracy();
            
            //update device model with new data
            d.imu.data.set(new_data);
        }

        vTaskDelay(5/portTICK_PERIOD_MS);
    }
}