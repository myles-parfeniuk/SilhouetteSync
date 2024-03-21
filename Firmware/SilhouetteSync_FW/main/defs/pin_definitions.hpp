#pragma once
#include "driver/gpio.h"

// IMU (SPI)
static constexpr gpio_num_t pin_imu_hint = GPIO_NUM_8; ///<IMU host interrupt pin
static constexpr gpio_num_t pin_imu_rst = GPIO_NUM_6;  ///<IMU reset pin
static constexpr gpio_num_t pin_imu_cs = GPIO_NUM_9;   ///<IMU SPI chip select pin
static constexpr gpio_num_t pin_imu_scl = GPIO_NUM_7;  ///<IMU SPI SCL pin
static constexpr gpio_num_t pin_imu_sda = GPIO_NUM_5; ///<IMU SPI MISO pin 
static constexpr gpio_num_t pin_imu_di = GPIO_NUM_4;   ///<IMU SPI MOSI pin

// User IO
static constexpr gpio_num_t pin_leds_di = GPIO_NUM_2; ///<Data out pin to WS281B LEDs (4 in series)
static constexpr gpio_num_t pin_user_sw = GPIO_NUM_21; ///<Power switch doubles as generic switch. Active high

// Dock UART
static constexpr gpio_num_t pin_dock_rx = GPIO_NUM_0; ///<ESP side UART RX pin from the dock
static constexpr gpio_num_t pin_dock_tx = GPIO_NUM_19; ///<ESP side UART TX pin to the dock

// Power Stuff
static constexpr gpio_num_t pin_charge_stat = GPIO_NUM_1; ///<LiPo charge IC status. Low when charging
static constexpr gpio_num_t pin_vbat_adc = GPIO_NUM_3; ///<Scaled battery voltage by 0.545
static constexpr gpio_num_t pin_pwr_src = GPIO_NUM_10; ///<Power source indicator. Low when USB powered
static constexpr gpio_num_t pin_buck_en = GPIO_NUM_20; ///<Hold high to enable main 3.3V rail while battery powered