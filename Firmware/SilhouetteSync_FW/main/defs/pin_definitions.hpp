#pragma once
#include "driver/gpio.h"

//display i2c
static constexpr gpio_num_t pin_i2c_sda = GPIO_NUM_21; ///<I2C SDA pin for OLED display
static constexpr gpio_num_t pin_i2c_scl = GPIO_NUM_22; ///<I2C SCLK pin for OLED display

//imu 
static constexpr gpio_num_t pin_imu_hint = GPIO_NUM_26; ///<IMU host interrupt pin
static constexpr gpio_num_t pin_imu_rst = GPIO_NUM_32; ///<IMU reset pin
static constexpr gpio_num_t pin_imu_cs = GPIO_NUM_33; ///<IMU SPI chip select pin
static constexpr gpio_num_t pin_imu_scl = GPIO_NUM_18; ///<IMU SPI SCL pin
static constexpr gpio_num_t pin_imu_sda = GPIO_NUM_19; ///<IMU SDA pin (esp32 MISO pin)
static constexpr gpio_num_t pin_imu_di = GPIO_NUM_23; ///<IMU DI pin (esp32 MOSI pin)


//nav_switch
static constexpr gpio_num_t pin_nav_up = GPIO_NUM_14;
static constexpr gpio_num_t pin_nav_enter = GPIO_NUM_25;
static constexpr gpio_num_t pin_nav_down = GPIO_NUM_27;
