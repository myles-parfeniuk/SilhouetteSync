#include "IMU.hpp"

IMU::IMU(Device& d)
    : d(d)
    , imu(bno08x_config_t(SPI2_HOST, pin_imu_di, pin_imu_sda, pin_imu_scl, pin_imu_cs, pin_imu_hint, pin_imu_rst, GPIO_NUM_NC, 2000000UL, false))
    , imu_state_event_group_hdl(xEventGroupCreate())
{
    // follow device struct to check for current sensor mode
    d.imu.state.follow(
        [this, &d](IMUState new_state)
        {
            switch (new_state)
            {
            case IMUState::sleep:
                // do nothing
                break;

            case IMUState::sample:
                ESP_LOGW(TAG, "SAMPLE STATE");
                xEventGroupClearBits(imu_state_event_group_hdl, ALL_IMU_STATE_BITS);
                xEventGroupSetBits(imu_state_event_group_hdl, SAMPLING_STATE_BIT);
                break;

            case IMUState::tare:
                ESP_LOGW(TAG, "TARE STATE");
                xEventGroupClearBits(imu_state_event_group_hdl, ALL_IMU_STATE_BITS);
                xEventGroupSetBits(imu_state_event_group_hdl, TARE_STATE_BIT);
                break;

            case IMUState::calibrate:
                ESP_LOGW(TAG, "CALIBRATE STATE");
                xEventGroupClearBits(imu_state_event_group_hdl, ALL_IMU_STATE_BITS);
                xEventGroupSetBits(imu_state_event_group_hdl, CALIBRATION_STATE_BIT);
                break;

            default:

                break;
            }
        },
        true);

    imu.initialize(); // initialize IMU unit

    // initialize imu game rotation vector and gyro
    imu.enable_game_rotation_vector(ROTATION_VECTOR_REPORT_PERIOD_US);
    // imu.enable_gyro(GYRO_REPORT_PERIOD_US);

    xTaskCreate(&imu_task_trampoline, "imu_task", 4096, this, 7, &imu_task_hdl);

    d.imu.state.set(IMUState::sample);
}

void IMU::imu_task_trampoline(void* arg)
{
    IMU* active_imu = (IMU*) arg;

    active_imu->imu_task();
}

void IMU::imu_task()
{

    while (1)
    {
        imu_state_bits = xEventGroupWaitBits(imu_state_event_group_hdl, ALL_IMU_STATE_BITS, pdFALSE, pdFALSE, portMAX_DELAY);

        if (imu_state_bits & SAMPLING_STATE_BIT)
            take_samples();
        else if (imu_state_bits & TARE_STATE_BIT)
            tare_imu();
        else if (imu_state_bits & CALIBRATION_STATE_BIT)
            calibrate_imu();
    }
}

void IMU::take_samples()
{
    imu_data_t new_data;
    imu.enable_game_rotation_vector(ROTATION_VECTOR_REPORT_PERIOD_US);
    // imu.enable_gyro(GYRO_REPORT_PERIOD_US);
    do
    {
        if (imu.data_available())
        {
            // update euler angle
            new_data.euler_heading.x = imu.get_roll();
            new_data.euler_heading.y = imu.get_pitch();
            new_data.euler_heading.z = imu.get_yaw();
            new_data.euler_heading.accuracy = imu.get_quat_accuracy();

            // update quaternion
            new_data.quaternion_heading.i = imu.get_quat_I();
            new_data.quaternion_heading.j = imu.get_quat_J();
            new_data.quaternion_heading.k = imu.get_quat_K();
            new_data.quaternion_heading.real = imu.get_quat_real();
            new_data.quaternion_heading.accuracy = imu.get_quat_accuracy();

            // update velocity
            new_data.velocity.x = imu.get_gyro_calibrated_velocity_X();
            new_data.velocity.y = imu.get_gyro_calibrated_velocity_Y();
            new_data.velocity.z = imu.get_gyro_calibrated_velocity_Z();
            new_data.velocity.accuracy = imu.get_gyro_accuracy();

            // update device model with new data
            d.imu.data.set(new_data);
        }

        imu_state_bits = xEventGroupWaitBits(imu_state_event_group_hdl, ALL_IMU_STATE_BITS, pdFALSE, pdFALSE, 0);

    } while (imu_state_bits & SAMPLING_STATE_BIT);
}

void IMU::calibrate_imu()
{
    d.imu.calibration_status.set(false);

    if (calibration_routine())
        d.imu.calibration_status.set(true);

    xEventGroupClearBits(imu_state_event_group_hdl, ALL_IMU_STATE_BITS);

    d.imu.state.set(IMUState::sample);
}

bool IMU::calibration_routine()
{
    imu_data_t new_data;
    float magf_x = 0;
    float magf_y = 0;
    float magf_z = 0;
    uint8_t magnetometer_accuracy = (uint8_t) IMUAccuracy::LOW;
    uint16_t high_accuracy = 0;
    uint16_t save_calibration_attempt = 0;

    // Enable dynamic calibration for accel, gyro, and mag
    imu.calibrate_all(); // Turn on cal for Accel, Gyro, and Mag

    // Enable Game Rotation Vector output
    imu.enable_game_rotation_vector(25); // Send data update every 25ms

    // Enable Magnetic Field output
    imu.enable_magnetometer(25); // Send data update every 25ms

    vTaskDelay(80 / portTICK_PERIOD_MS);

    while (1)
    {
        if (imu.data_available())
        {
            magf_x = imu.get_magf_X();
            magf_y = imu.get_magf_Y();
            magf_z = imu.get_magf_Z();

            magnetometer_accuracy = imu.get_magf_accuracy();
            new_data.quaternion_heading.i = imu.get_quat_I();
            new_data.quaternion_heading.j = imu.get_quat_J();
            new_data.quaternion_heading.k = imu.get_quat_K();
            new_data.quaternion_heading.real = imu.get_quat_real();
            new_data.quaternion_heading.accuracy = imu.get_quat_accuracy();

            d.imu.data.set(new_data);
            ESP_LOGI(TAG, "Magnetometer: x: %.3f y: %.3f z: %.3f, accuracy: %d", magf_x, magf_y, magf_z, magnetometer_accuracy);

            if ((magnetometer_accuracy >= (uint8_t) IMUAccuracy::MED) && (new_data.quaternion_heading.accuracy == (uint8_t) IMUAccuracy::HIGH))
                high_accuracy++;
            else
                high_accuracy = 0;

            if (high_accuracy > 100)
            {
                imu.save_calibration();
                imu.request_calibration_status();

                save_calibration_attempt = 0;

                while (save_calibration_attempt < 20)
                {

                    if (imu.data_available())
                    {
                        if (imu.calibration_complete())
                        {
                            ESP_LOGW(TAG, "Calibration data successfully stored.");
                            imu.disable_magnetometer();
                            return true;
                        }
                        else
                        {
                            imu.save_calibration();
                            imu.request_calibration_status();
                            save_calibration_attempt++;
                        }

                        vTaskDelay(1 / portTICK_PERIOD_MS);
                    }
                }

                if (save_calibration_attempt >= 20)
                    ESP_LOGE(TAG, "Calibration data failed to store.");

                imu.disable_magnetometer();
                return false;
            }
        }

        vTaskDelay(5 / portTICK_PERIOD_MS);
    }
}

void IMU::tare_imu()
{
    imu.tare_now(BNO08x::TARE_AXIS_ALL, BNO08x::TARE_GAME_ROTATION_VECTOR);
    xEventGroupClearBits(imu_state_event_group_hdl, ALL_IMU_STATE_BITS);
    d.imu.state.set(IMUState::sample);
}