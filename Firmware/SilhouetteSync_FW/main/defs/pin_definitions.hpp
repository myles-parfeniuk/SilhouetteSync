#pragma once
#include "driver/gpio.h"

//IMU (SPI)
static constexpr gpio_num_t pin_imu_hint = GPIO_NUM_6; ///<IMU host interrupt pin
static constexpr gpio_num_t pin_imu_rst = GPIO_NUM_7; ///<IMU reset pin
static constexpr gpio_num_t pin_imu_cs = GPIO_NUM_5; ///<IMU SPI chip select pin
static constexpr gpio_num_t pin_imu_scl = GPIO_NUM_1; ///<IMU SPI SCL pin
static constexpr gpio_num_t pin_imu_sda = GPIO_NUM_10; ///<IMU SDA pin (esp32 MISO pin)
static constexpr gpio_num_t pin_imu_di = GPIO_NUM_4; ///<IMU DI pin (esp32 MOSI pin)

//LED Indicator
static constexpr gpio_num_t pin_leds_di = GPIO_NUM_8; ///<Data in pin for WS281B LED strip