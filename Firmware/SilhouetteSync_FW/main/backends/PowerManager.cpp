#include "PowerManager.hpp"

PowerManager::PowerManager(Device& d)
    : d(d)
    , power_management_evt_group_hdl(xEventGroupCreate())
{
    // follow to switch events and listen for long press to shutdown device accordingly
    d.user_sw.follow(
            [this](SwitchEvents new_event)
            {
                if (new_event == SwitchEvents::long_press)
                {

                    if (this->d.power_state.get() == PowerStates::normal_operation)
                    {
                        gpio_set_level(pin_buck_en, 0); // shutdown buck if switch is long pressed in normal operation mode
                        this->d.power_state.set(PowerStates::shutdown);
                    }
                    else if (this->d.power_state.get() == PowerStates::low_power)
                    {
                        gpio_set_level(pin_buck_en, 1); // enable buck if switch is long pressed in low power mode
                        this->d.power_state.set(PowerStates::boot);
                    }
                }
                else if (new_event == SwitchEvents::released)
                {
                    if (this->d.power_state.get() == PowerStates::shutdown)
                        this->d.power_state.set(PowerStates::low_power);
                    else if (this->d.power_state.get() == PowerStates::boot)
                        this->d.power_state.set(PowerStates::normal_operation);
                }
            },
            true);

    init_gpio();
    set_buck_en_on_boot();
    set_power_source_state(gpio_get_level(pin_pwr_or_state), gpio_get_level(pin_charge_state));
    xTaskCreate(&power_management_task_trampoline, "power_management_task", 4096, this, 1, &power_management_task_hdl);
};

void PowerManager::init_gpio()
{
    gpio_config_t buck_en_conf = {
            .pin_bit_mask = (1ULL << (uint64_t) pin_buck_en),
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
    };

    gpio_config(&buck_en_conf); // configure buck en gpio pin

    gpio_config_t pwr_or_state_conf = {
            .pin_bit_mask = (1ULL << (uint64_t) pin_pwr_or_state),
            .mode = GPIO_MODE_INPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_ANYEDGE,
    };

    gpio_config(&pwr_or_state_conf); // configure power or state gpio pin

    gpio_config_t charge_state_conf = {
            .pin_bit_mask = (1ULL << (uint64_t) pin_charge_state),
            .mode = GPIO_MODE_INPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_ANYEDGE,
    };

    gpio_config(&charge_state_conf); // configure charge state gpio pin

    gpio_install_isr_service(0); // install ISR service

    // add ISR handlers
    ESP_ERROR_CHECK(gpio_isr_handler_add(pin_pwr_or_state, pwr_or_state_ISR, (void*) this)); // add or state isr handler
    ESP_ERROR_CHECK(gpio_isr_handler_add(pin_charge_state, charge_state_ISR, (void*) this)); // add or state isr handler

    // enable interrupts
    gpio_intr_enable(pin_pwr_or_state);
    gpio_intr_enable(pin_charge_state);
}

void PowerManager::set_buck_en_on_boot()
{
    int USB_connection_state = gpio_get_level(pin_pwr_or_state);

    if (USB_connection_state == (int) PWROrState::battery_powered)
    {
        gpio_set_level(pin_buck_en, 1);
        d.power_state.set(PowerStates::normal_operation);
    }
    else
    {
        gpio_set_level(pin_buck_en, 0);
    }
}

void PowerManager::set_power_source_state(int or_state, int charge_state)
{
    if (or_state)
        d.power_source_state.set(PowerSourceStates::battery_powered);
    else if (!or_state && charge_state)
        d.power_source_state.set(PowerSourceStates::USB_powered_fully_charged);
    else if (!or_state && !charge_state)
        d.power_source_state.set(PowerSourceStates::USB_powered_charging);
    else
        ESP_LOGE(TAG, "Invalid power state."); 
}

void PowerManager::power_management_task_trampoline(void* arg)
{
    PowerManager* local_power_manager = (PowerManager*) arg;
    local_power_manager->power_management_task();
}

void PowerManager::power_management_task()
{
    EventBits_t event_bits;

    while (1)
    {
        event_bits = xEventGroupWaitBits(power_management_evt_group_hdl, ALL_POWER_MANAGEMENT_EVT_BITS, pdFALSE, pdFALSE, portMAX_DELAY);

        if (event_bits & OR_STATE_EVT_BIT)
        {
            vTaskDelay(400 / portTICK_PERIOD_MS);
            // ESP_LOGI(TAG, "Or State:  %d", gpio_get_level(pin_pwr_or_state));
            set_power_source_state(gpio_get_level(pin_pwr_or_state), gpio_get_level(pin_charge_state));

            xEventGroupClearBits(power_management_evt_group_hdl, OR_STATE_EVT_BIT);
            gpio_intr_enable(pin_pwr_or_state);
        }
        else if (event_bits & CHARGE_STATE_EVT_BIT)
        {
            vTaskDelay(400 / portTICK_PERIOD_MS);
            // ESP_LOGI(TAG, "Charge State:  %d", gpio_get_level(pin_charge_state));
            set_power_source_state(gpio_get_level(pin_pwr_or_state), gpio_get_level(pin_charge_state));

            xEventGroupClearBits(power_management_evt_group_hdl, CHARGE_STATE_EVT_BIT);
            gpio_intr_enable(pin_charge_state);
        }
    }
}

void IRAM_ATTR PowerManager::pwr_or_state_ISR(void* arg)
{
    BaseType_t xHighPriorityTaskWoken = pdFALSE;
    PowerManager* local_power_manager = (PowerManager*) arg;

    gpio_intr_disable(pin_pwr_or_state);
    xEventGroupSetBitsFromISR(local_power_manager->power_management_evt_group_hdl, OR_STATE_EVT_BIT, &xHighPriorityTaskWoken);

    if (xHighPriorityTaskWoken == pdTRUE)
        portYIELD_FROM_ISR(xHighPriorityTaskWoken);
}

void IRAM_ATTR PowerManager::charge_state_ISR(void* arg)
{
    BaseType_t xHighPriorityTaskWoken = pdFALSE;
    PowerManager* local_power_manager = (PowerManager*) arg;

    gpio_intr_disable(pin_charge_state);
    xEventGroupSetBitsFromISR(local_power_manager->power_management_evt_group_hdl, CHARGE_STATE_EVT_BIT, &xHighPriorityTaskWoken);

    if (xHighPriorityTaskWoken == pdTRUE)
        portYIELD_FROM_ISR(xHighPriorityTaskWoken);
}