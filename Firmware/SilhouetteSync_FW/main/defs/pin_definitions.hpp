#pragma once
#include "driver/gpio.h"

//i2c
static constexpr gpio_num_t pin_i2c_sda = GPIO_NUM_21;
static constexpr gpio_num_t pin_i2c_scl = GPIO_NUM_22;

//nav_switch
static constexpr gpio_num_t pin_nav_up = GPIO_NUM_26;
static constexpr gpio_num_t pin_nav_down = GPIO_NUM_25;
