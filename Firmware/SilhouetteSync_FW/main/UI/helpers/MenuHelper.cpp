#include "MenuHelper.hpp"

SemaphoreHandle_t MenuHelper::display_buffer_mutex = xSemaphoreCreateMutex();
DataControl::CallAlways<MenuCodes> MenuHelper::active_menu(MenuCodes::main_menu_sel);

void MenuHelper::lock_display_buffer()
{
    xSemaphoreTake(display_buffer_mutex, portMAX_DELAY);
}

void MenuHelper::unlock_display_buffer()
{
    xSemaphoreGive(display_buffer_mutex);
}
