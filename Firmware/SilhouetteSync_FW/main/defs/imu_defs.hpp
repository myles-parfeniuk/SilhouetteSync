#pragma once
#include <stdint.h>
    /** 
    *   @brief  The different states the IMU can be in
    */
    enum class IMUState 
    {
        sleep, ///<sleep state (do nothing)
        sample, ///<sample stake, continuously update device model with new IMU data as it's received
        calibrate, ///<calibration state, run calibration routine
        max_modes
    };

    /** 
    *   @brief  Representation of euler angle reading from IMU in radians (roll, pitch, yaw)
    */
    typedef struct
    {
        float x= 0.0; ///<rotation about x (roll) in radians
        float y = 0.0; ///<rotation about y (pitch) in radians
        float z = 0.0; ///<rotation about z (pitch) in radians
        uint8_t accuracy = 0; ///<accuracy of reading according to IMU (number 0 through 3)
    } euler_angle_t; 

    /** 
    *   @brief  Representation of quaternion reading from IMU.
    */
    typedef struct
    {
        float i = 0.0;  ///<quaternion i component
        float j = 0.0;  ///<quaternion j component
        float k = 0.0; ///<quaternion k component
        float real = 0.0; ///<quaternion real component
        uint8_t accuracy = 0; ///<accuracy of reading according to IMU (number 0 through 3)
    } quaternion_t; 

   /** 
    *   @brief  Representation of velocity reading from IMU in Rad/s
    */
    typedef struct
    {
        float x = 0.0;  ///<x axis angular velocity in Rad/s
        float y = 0.0;  ///<y axis angular velocity in Rad/s
        float z = 0.0;  ///<z axis angular velocity in Rad/s
        uint8_t accuracy = 0; ///<accuracy of reading according to IMU (number 0 through 3)
    } velocity_t; 

    /** 
    *   @brief  Representation of all data SilhouetteSync reads from BNO085 IMU. 
    */
    typedef struct 
    {
        //initialize with respective default constructors
        euler_angle_t euler_heading = euler_angle_t(); ///<euler angle absolute heading (pitch, roll yaw) in radians, returned by BN085 IMU. 
        quaternion_t quaternion_heading = quaternion_t(); ///<quaternion absolute heading returned by BN085 IMU. 
        velocity_t velocity = velocity_t(); ///<velocity in Rad/s, returned by BNO085 IMU. 
    } imu_data_t;


