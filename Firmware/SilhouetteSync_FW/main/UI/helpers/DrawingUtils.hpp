#pragma once
//third party includes
#include "u8g2.h"

#define STR_ALIGN_CENTER(str,disp) ((u8g2_GetDisplayWidth(disp)-u8g2_GetUTF8Width(disp,str))/2) //align string to horizontal center macro 
#define STR_ALIGN_CENTER_REF(str, disp, x_ref) ((u8g2_GetDisplayWidth(disp)-u8g2_GetUTF8Width(disp,str) + (2*x_ref))/2)

class DrawingUtils
{
    public:
    static void draw_util_eraseArea(u8g2_t *disp, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h);
    static void draw_progress_bar(u8g2_t *disp, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h, uint16_t current_val, uint16_t min_val, uint16_t max_val);
    static void draw_progress_bar_with_label(u8g2_t *disp, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h, uint16_t current_val, uint16_t min_val, uint16_t max_val, char *label);
};