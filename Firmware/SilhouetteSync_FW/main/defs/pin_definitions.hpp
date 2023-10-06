#pragma once
#include "driver/gpio.h"

//display i2c
static constexpr gpio_num_t pin_i2c_sda = GPIO_NUM_21;
static constexpr gpio_num_t pin_i2c_scl = GPIO_NUM_22;

//imu uart
static constexpr gpio_num_t pin_uart_rx = GPIO_NUM_17; //connect to IMU SCL pin 
static constexpr gpio_num_t pin_uart_tx = GPIO_NUM_16; //connect to IMU SDA pin

//nav_switch
static constexpr gpio_num_t pin_nav_up = GPIO_NUM_26;
static constexpr gpio_num_t pin_nav_down = GPIO_NUM_25;
