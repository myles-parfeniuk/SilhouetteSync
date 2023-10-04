#pragma once
//esp-idf component includes
#include "esp_log.h"
#include "U8G2Hal.hpp"
//component includes


#define OLED_BASE_ADDR 0x3C

void displayInit(u8g2_t * disp);

