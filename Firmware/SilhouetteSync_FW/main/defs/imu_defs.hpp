#pragma once

    /** 
    *   @brief  the different states the IMU can be in
    */
    enum class IMUState 
    {
        sleep,
        sample,
        calibrate,
        max_modes
    };

