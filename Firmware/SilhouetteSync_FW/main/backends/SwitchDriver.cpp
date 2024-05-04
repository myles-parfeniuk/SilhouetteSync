#include "SwitchDriver.hpp"

SwitchDriver::SwitchDriver(Device& d)
    : d(d)
    , double_tap_counter(0)
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

    esp_timer_create_args_t double_tap_reset_timer_args = {
            .callback = double_tap_reset_timer_cb_trampoline, .arg = this, .name = "DoubleTapResetTimer"};
    esp_timer_create(&double_tap_reset_timer_args, &double_tap_reset_timer_hdl);
}

void SwitchDriver::switch_task()
{
    handle_boot();

    // enable interrupts
    gpio_intr_enable(pin_user_sw);

    while (1)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY); // block until notified by switch ISR
        if (!quick_press_check())
        {
            released_check();
        }
        else
        {
            esp_timer_start_once(double_tap_reset_timer_hdl, 270000ULL);
            double_tap_counter++;
            if (double_tap_counter > 1)
                generate_double_tap_event();
        }
    }
}

void SwitchDriver::switch_task_trampoline(void* arg)
{
    SwitchDriver* local_switch_driver = (SwitchDriver*) arg; // cast to SwitchDriver pointer
    local_switch_driver->switch_task();                      // launch switch task
}

void SwitchDriver::handle_boot()
{
    int64_t start_time = esp_timer_get_time();
    int64_t elapsed_time = 0;
    bool held = true;

    do
    {
        if (gpio_get_level(pin_user_sw))
            held = true;
        else
            held = false;

        elapsed_time = esp_timer_get_time() - start_time;

        vTaskDelay(5 / portTICK_PERIOD_MS);

    } while ((elapsed_time < 200000) && held);

    // if switch is being held, valid boot state
    if (held)
    {
        d.power_state.set(PowerStates::boot);
        generate_long_press_event();
        released_check();
    }
    else
    {
        if (d.power_source_state.get() == (PowerSourceStates::battery_powered))
        {
            d.power_state.set(PowerStates::shutdown);
            generate_long_press_event();
            generate_released_event();
        }
        else
        {
            d.power_state.set(PowerStates::low_power);
            gpio_set_level(pin_buck_en, 0);
        }
    }
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

void SwitchDriver::generate_double_tap_event()
{
    d.user_sw.set(SwitchEvents::double_tap);
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

void SwitchDriver::double_tap_reset_timer_cb_trampoline(void* arg)
{
    SwitchDriver* local_switch_driver = (SwitchDriver*) arg;

    local_switch_driver->double_tap_reset_timer_cb();
}

void SwitchDriver::double_tap_reset_timer_cb()
{
    double_tap_counter = 0;
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