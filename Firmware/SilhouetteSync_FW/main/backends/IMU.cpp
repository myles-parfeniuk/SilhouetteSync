#include "IMU.hpp"

//https://cdn-shop.adafruit.com/datasheets/BST_BNO055_DS000_12.pdf
IMU::IMU(Device &d):
d(d),
imu_unit(UART_NUM_1, pin_uart_rx, pin_uart_tx),
take_samples(false),
calibrating(false)
{
    //follow device struct to check for current sensor mode
    d.imu.state.follow([this, &d](IMUState new_state){
        switch(new_state)
        {
            case IMUState::sleep:
                if(this->take_samples)
                    this->take_samples = false; 
            break;

            case IMUState::sample:
                if(!this->take_samples)
                    this->take_samples = true; 
                vTaskResume(this->sample_task_hdl);
            break;

            case IMUState::calibrate:
                if(!this->calibrating)
                    this->calibrating = true; 

                if(this->take_samples)
                    this->take_samples = false; 
                else
                    vTaskResume(this->sample_task_hdl);
            break;

            default:

            break;
        }
    }, true);

    //setup imu unit
    try 
    {
        imu_unit.begin(); //initialize UART and set BNO055 operation mode to config
        imu_unit.enableExternalCrystal(); //enable BNO055 external oscillator 
        imu_unit.setOprModeNdof(); //set operation mode to NDOF (fusion mode with 9 degrees of freedom, see section 3.3.3.5 of datasheet)
        
        ESP_LOGI(TAG, "Setup succeeded..");
    }
    catch(BNO055BaseException& ex)
    {
        ESP_LOGE(TAG, "Setup failed, Error: %s", ex.what());
    }
    catch(std::exception& ex)
    {
        ESP_LOGE(TAG, "Setup failed, Error: %s", ex.what());
    }

    //try some basic reads to ensure imu unit is working
    try
    {
        //read and display bootloader and firmware revision 
        int16_t sw = imu_unit.getSWRevision();
        uint8_t bl_rev = imu_unit.getBootloaderRevision(); 
        ESP_LOGI(TAG, "SW rev: %d, bootloader rev: %u", sw, bl_rev);

        //do self test and display results
        bno055_self_test_result_t res = imu_unit.getSelfTestResult();
        ESP_LOGI(TAG, "Self-Test Results: MCU: %u, GYR:%u, MAG:%u, ACC: %u", res.mcuState, res.gyrState, res.magState, res.accState);

        //check calibration
        bno055_calibration_t cal = imu_unit.getCalibration();
        ESP_LOGI(TAG, "Calibration Status (0 = not calibrated, 3 = fully calibrated): %u GYRO: %u ACC:%u MAG:%u", cal.sys, cal.gyro, cal.accel, cal.mag);
    }
    catch(BNO055BaseException& ex)
    {
        ESP_LOGE(TAG, "Error: %s", ex.what());
    }
    catch(std::exception& ex)
    {
        ESP_LOGE(TAG, "Error: %s", ex.what());
    }

    xTaskCreate(&sampling_task_trampoline, "imu_sample_task", 4096, this, 5, &sample_task_hdl);
}

void IMU::calibrate_imu()
{

        while (1) {
        try {
            // Calibration 3 = fully calibrated, 0 = not calibrated
            bno055_calibration_t cal = imu_unit.getCalibration();
            bno055_vector_t v = imu_unit.getVectorEuler();
            ESP_LOGI(TAG, "Euler: X: %.1f Y: %.1f Z: %.1f || Calibration SYS: %u GYRO: %u ACC:%u MAG:%u", v.x, v.y, v.z, cal.sys,
                     cal.gyro, cal.accel, cal.mag);
            if (cal.gyro == 3 && cal.accel == 3 && cal.mag == 3) {
                ESP_LOGI(TAG, "Fully Calibrated.");
                imu_unit.setOprModeConfig();                         // Change to OPR_MODE
                bno055_offsets_t txt = imu_unit.getSensorOffsets();  // NOTE: this must be executed in CONFIG_MODE

                //we should consider storing these offsets such that we don't need to calibrate every time
                //the magnometer unfortunately must be calibrated every time according to BNO055 datasheet
                ESP_LOGI(TAG,
                         "\nOffsets:\nAccel: X:%d, Y:%d, Z:%d;\nMag: X:%d, Y:%d, Z:%d;\nGyro: X:%d, Y:%d, Z:%d;\nAccelRadius: "
                         "%d;\nMagRadius: %d;\n",
                         txt.accelOffsetX, txt.accelOffsetY, txt.accelOffsetZ, txt.magOffsetX, txt.magOffsetY, txt.magOffsetZ,
                         txt.gyroOffsetX, txt.gyroOffsetY, txt.gyroOffsetZ, txt.accelRadius, txt.magRadius);
                
                d.imu.calibration_status.set(true); //set calibration status as true

                break;
            }
        } catch (BNO055BaseException& ex) {
            ESP_LOGE(TAG, "Something bad happened: %s", ex.what());
            return;
        } catch (std::exception& ex) {
            ESP_LOGE(TAG, "Something bad happened: %s", ex.what());
        }

        vTaskDelay(100 / portTICK_PERIOD_MS);  // in fusion mode max output rate is 100hz (actual rate: 100ms (10hz))
    }

}

void IMU::sampling_task_trampoline(void *imu_unit)
{   
    IMU * active_imu_unit = (IMU *)imu_unit;

    active_imu_unit->sampling_task(); 
}

void IMU::sampling_task()
{
    while(1)
    {

        switch(d.imu.state.get())
        {
            case IMUState::sleep:
                vTaskSuspend(NULL); //self suspend sampling task 
            break;

            case IMUState::sample:
                d.imu.vector.set(imu_unit.getVectorEuler()); 
            break;

            case IMUState::calibrate:
                calibrate_imu();
                d.imu.state.set(IMUState::sleep);
            break;

            default:

            break; 
            
        }
        vTaskDelay(100/portTICK_PERIOD_MS);
    }
}