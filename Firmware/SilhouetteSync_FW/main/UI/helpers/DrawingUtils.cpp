#include "DrawingUtils.hpp"

void DrawingUtils::draw_util_eraseArea(u8g2_t *disp, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h)
{
    u8g2_SetDrawColor(disp, 0);
    u8g2_DrawBox(disp, x, y, w, h);
    u8g2_SetDrawColor(disp, 1);
}

void DrawingUtils:: draw_progress_bar(u8g2_t *disp, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h, uint16_t current_val, uint16_t min_val, uint16_t max_val)
{
    uint16_t progress = (current_val - min_val) * w / (max_val - min_val) + min_val;

    u8g2_DrawFrame(disp, x, y, w, h);
    u8g2_DrawBox(disp, x, y, progress, h);
}

void DrawingUtils::draw_progress_bar_with_label(u8g2_t *disp, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h, uint16_t current_val, uint16_t min_val, uint16_t max_val, char *label)
{
    uint16_t progress = (current_val - min_val) * w / (max_val - min_val) + min_val;
    u8g2_DrawFrame(disp, x, y, w, h);
    u8g2_DrawBox(disp, x, y, progress, h);
    u8g2_SetDrawColor(disp, 2);
    u8g2_DrawStr(disp, STR_ALIGN_CENTER(label, disp), y+(h-2), label);
    u8g2_SetDrawColor(disp, 1);
}
    
