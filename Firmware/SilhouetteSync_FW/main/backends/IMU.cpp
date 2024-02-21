#include "IMU.hpp"

IMU::IMU(Device &d):
d(d),
imu_state_event_group_hdl(xEventGroupCreate())
{
    //follow device struct to check for current sensor mode
    d.imu.state.follow([this, &d](IMUState new_state){
        switch(new_state)
        {
            case IMUState::sleep:
                //do nothing
            break;

            case IMUState::sample:
                xEventGroupClearBits(imu_state_event_group_hdl, ALL_IMU_STATE_BITS);
                xEventGroupSetBits(imu_state_event_group_hdl, SAMPLING_STATE_BIT);
            break;

            case IMUState::calibrate:
                xEventGroupClearBits(imu_state_event_group_hdl, ALL_IMU_STATE_BITS);
                xEventGroupSetBits(imu_state_event_group_hdl, CALIBRATION_STATE_BIT);
            break;

            default:

            break;
        }
    });

    imu.initialize(); //initialize IMU unit

    //initialize imu game rotation vector and gyro
    imu.enable_game_rotation_vector(100); 
    imu.enable_gyro(150);

    xTaskCreate(&imu_task_trampoline, "imu_task", 4096, this, 5, &imu_task_hdl);
}


void IMU::imu_task_trampoline(void *arg)
{   
    IMU * active_imu = (IMU *)arg;

    active_imu->imu_task(); 
}

void IMU::imu_task()
{

    while(1)
    {
        imu_state_bits = xEventGroupWaitBits(imu_state_event_group_hdl, ALL_IMU_STATE_BITS, pdFALSE, pdFALSE, portMAX_DELAY);
        

        if(imu_state_bits & SAMPLING_STATE_BIT)
            take_samples(); 
        else if(imu_state_bits & CALIBRATION_STATE_BIT)
            calibrate_imu(); 
      
        
    }

}

void IMU::take_samples()
{
        imu_data_t new_data;

        do
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

            imu_state_bits = xEventGroupWaitBits(imu_state_event_group_hdl, ALL_IMU_STATE_BITS, pdFALSE, pdFALSE, 0);

        } while (imu_state_bits & SAMPLING_STATE_BIT);

}

void IMU::calibrate_imu()
{
    if(imu.run_full_calibration_routine())
    {
        d.imu.calibration_status.set(true); 
    }
    xEventGroupClearBits(imu_state_event_group_hdl, ALL_IMU_STATE_BITS);
}

void IMU::wait_for_calibration()
{
    while(!d.imu.calibration_status.get())
    {
        vTaskDelay(50/portTICK_PERIOD_MS);
    }
}