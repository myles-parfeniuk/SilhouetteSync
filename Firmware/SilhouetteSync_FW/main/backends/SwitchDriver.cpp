#include "SwitchDriver.hpp"

SwitchDriver::SwitchDriver(Device& d)
    : d(d)
{
    init_gpio();
    xTaskCreate(&switch_task_trampoline, "switch_task", 4096, this, 2, &switch_task_hdl);
}

void SwitchDriver::init_gpio()
{
    gpio_config_t button_conf = {
            .pin_bit_mask = (1ULL << (uint64_t) pin_user_sw),
            .mode = GPIO_MODE_INPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_POSEDGE,
    };

    // configure switch gpio pin
    ESP_ERROR_CHECK(gpio_config(&button_conf));

    // add interrupt handler
    ESP_ERROR_CHECK(gpio_isr_handler_add(pin_user_sw, switch_ISR, (void*) this));
}

void SwitchDriver::switch_task()
{
    // enable interrupts
    gpio_intr_enable(pin_user_sw);

    while (1)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY); // block until notified by switch ISR
        if (!quick_press_check())
            released_check();
    }
}

void SwitchDriver::switch_task_trampoline(void* arg)
{
    SwitchDriver* local_switch_driver = (SwitchDriver*) arg; // cast to SwitchDriver pointer
    local_switch_driver->switch_task();                      // launch switch task
}

bool SwitchDriver::quick_press_check()
{
    int64_t start_time = esp_timer_get_time();
    do
    {
        vTaskDelay(25 / portTICK_PERIOD_MS);

        if ((esp_timer_get_time() - start_time) > LONG_PRESS_EVT_TIME_US)
        {
            generate_long_press_event(); // generate long press event
            return false;                // long press event occured, return false
        }

    } while (gpio_get_level(pin_user_sw));

    generate_quick_press_event(); // generate quick press event
    gpio_intr_enable(pin_user_sw);
    return true; // quick press event occurred, return true
}

void SwitchDriver::released_check()
{
    do
    {
        vTaskDelay(HELD_EVT_TIME_MS / portTICK_PERIOD_MS);

        generate_held_event();

    } while (gpio_get_level(pin_user_sw));

    generate_released_event();
    gpio_intr_enable(pin_user_sw);
}

void SwitchDriver::generate_quick_press_event()
{
    d.user_sw.set(SwitchEvents::quick_press);
}

void SwitchDriver::generate_long_press_event()
{
    d.user_sw.set(SwitchEvents::long_press);
}

void SwitchDriver::generate_held_event()
{
    d.user_sw.set(SwitchEvents::held);
}
void SwitchDriver::generate_released_event()
{
    d.user_sw.set(SwitchEvents::released);
}

void IRAM_ATTR SwitchDriver::switch_ISR(void* arg)
{
    BaseType_t xHighPriorityTaskWoken = pdFALSE;
    SwitchDriver* local_switch_driver = (SwitchDriver*) arg;

    gpio_intr_disable(pin_user_sw);

    vTaskNotifyGiveFromISR(local_switch_driver->switch_task_hdl, &xHighPriorityTaskWoken); // notify button task button input was detected

    if (xHighPriorityTaskWoken == pdTRUE)
        portYIELD_FROM_ISR(xHighPriorityTaskWoken);
}