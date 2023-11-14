#include "NavSwitch.hpp"

NavSwitch::NavSwitch(Device &d):
detected_input(nav_input_t::enter),
d(d)
{
    gpio_config_t gpio_conf; 

    gpio_conf.pin_bit_mask = (0x1 << pin_nav_up) | (0x1 << pin_nav_down) | (0x1 << pin_nav_enter); //assign nav switch gpio numbers
    gpio_conf.mode = GPIO_MODE_INPUT; //set gpio mode as input
    gpio_conf.intr_type = GPIO_INTR_NEGEDGE; //set negative edge interrupt
    //internal pullups/pulldowns diabled
    gpio_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;

    gpio_config(&gpio_conf); //configure gpio pins

    gpio_install_isr_service(0);
    ESP_ERROR_CHECK(gpio_isr_handler_add(pin_nav_up, up_handler_ISR, (void *)this)); //add up handler ISR
    ESP_ERROR_CHECK(gpio_isr_handler_add(pin_nav_down, down_handler_ISR, (void *)this)); //add down handler ISR
    ESP_ERROR_CHECK(gpio_isr_handler_add(pin_nav_enter, enter_handler_ISR, (void *)this)); //add enter handler ISR

    nav_switch_task_hdl = NULL; 
    xTaskCreate(&nav_switch_task_trampoline, "nav_switch_task", 4096, this, 5, &nav_switch_task_hdl); //launch nav switch task
}


void NavSwitch::nav_switch_task_trampoline(void *arg)
{
    NavSwitch *local_switch = (NavSwitch *)arg; //cast to nav switch to pointer

    local_switch->nav_switch_task(); //launch nav switch task
}

void NavSwitch::nav_switch_task()
{
    while(1)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY); //block until notified by respective GPIO ISR

        vTaskDelay(25/portTICK_PERIOD_MS); //debounce press

        //check for press type, if nav input is not released (long press) then check for release event
        if(!press_check())
            released_check();   

        //re-enable all nav switch interrupts
        gpio_intr_enable(pin_nav_up);
        gpio_intr_enable(pin_nav_down);
        gpio_intr_enable(pin_nav_enter);
    }
}



bool NavSwitch::press_check()
{

    int64_t time_stamp = esp_timer_get_time();
    bool press_check = false;

    //press check: look for long-press or quick-press event
    while(!press_check)
    {
        //if nav input released
        if(!get_nav_input_level())
        {
            //generate quick press event and exit long-press check, and skip release check
            press_check = true;
            generate_quick_press_evt(); //generate quick press event
            vTaskDelay(25/portTICK_PERIOD_MS); //release debounce
            return true; 
        } 
        //long-press event time exceeded
        else if((esp_timer_get_time() - time_stamp) > long_press_evt_time)
        {
            //generate long press event and exit long-press check, progress to release check
            press_check = true; 
            generate_long_press_evt(); //generate long press event
        }

        vTaskDelay(15/portTICK_PERIOD_MS);
    }

    return false;
}
  

void NavSwitch::released_check()
{
    bool released_check = false;
    int64_t time_stamp = esp_timer_get_time(); 

    //release check: generate held events until release event
    while(!released_check)
    {
        //if nav input released
        if(!get_nav_input_level())
        {
            //generate release event and exit release check
            released_check = true;
            generate_released_evt();
            vTaskDelay(25/portTICK_PERIOD_MS); //release debounce
        }
        //if held event generation time exceeded
        else if((esp_timer_get_time() - time_stamp) > held_evt_time)
        {
            //reset time stamp generate held event
            time_stamp = esp_timer_get_time(); 
            generate_held_evt();
                        
        }

        vTaskDelay(15/portTICK_PERIOD_MS); //delay for idle task
    }
}

bool NavSwitch::get_nav_input_level()
{
        switch(detected_input)
        {
            case nav_input_t::up:
                return !gpio_get_level(pin_nav_up); 
            break;

            case nav_input_t::down:
                return !gpio_get_level(pin_nav_down); 
            break;

            case nav_input_t::enter:
                return !gpio_get_level(pin_nav_enter); 
            break;

            default:
                return false; 
            break;
        }

}

void NavSwitch::generate_quick_press_evt()
{
    switch(detected_input)
    {
        case nav_input_t::up:
            d.nav_switch.up.set(nav_switch_evt_t::quick_press);
        break; 

        case nav_input_t::down: 
            d.nav_switch.down.set(nav_switch_evt_t::quick_press);
        break; 

        case nav_input_t::enter:
            d.nav_switch.enter.set(nav_switch_evt_t::quick_press);
        break; 

        default: 

        break; 
    }
}

void NavSwitch::generate_long_press_evt()
{
    switch(detected_input)
    {
        case nav_input_t::up:
            d.nav_switch.up.set(nav_switch_evt_t::long_press);
        break; 

        case nav_input_t::down: 
            d.nav_switch.down.set(nav_switch_evt_t::long_press);
        break; 

        case nav_input_t::enter:
            d.nav_switch.enter.set(nav_switch_evt_t::long_press);
        break; 

        default: 

        break; 
    }
}

void NavSwitch::generate_held_evt()
{
    switch(detected_input)
    {
        case nav_input_t::up:
            d.nav_switch.up.set(nav_switch_evt_t::held);
        break; 

        case nav_input_t::down: 
            d.nav_switch.down.set(nav_switch_evt_t::held);
        break; 

        case nav_input_t::enter:
            d.nav_switch.enter.set(nav_switch_evt_t::held);
        break; 

        default: 

        break; 
    }
}

void NavSwitch::generate_released_evt()
{
    switch(detected_input)
    {
        case nav_input_t::up:
            d.nav_switch.up.set(nav_switch_evt_t::released);
        break; 

        case nav_input_t::down: 
            d.nav_switch.down.set(nav_switch_evt_t::released);
        break; 

        case nav_input_t::enter:
            d.nav_switch.enter.set(nav_switch_evt_t::released);
        break; 

        default: 

        break; 
    }
}

void IRAM_ATTR NavSwitch::up_handler_ISR(void *arg)
{
    BaseType_t xHighPriorityTaskWoken = pdFALSE;
    NavSwitch *local_switch = (NavSwitch *)arg;
    local_switch->detected_input = nav_input_t::up; //set input type 

    //disable all nav switch interrupts
    gpio_intr_disable(pin_nav_up);
    gpio_intr_disable(pin_nav_down);
    gpio_intr_disable(pin_nav_enter);

    vTaskNotifyGiveFromISR(local_switch->nav_switch_task_hdl, &xHighPriorityTaskWoken); //notify nav switch task nav switch input was detected
    portYIELD_FROM_ISR(xHighPriorityTaskWoken); //perform context switch if necessary
    
    
}

void IRAM_ATTR NavSwitch::down_handler_ISR(void *arg)
{
    BaseType_t xHighPriorityTaskWoken = pdFALSE;
    NavSwitch *local_switch = (NavSwitch *)arg;
    local_switch->detected_input = nav_input_t::down; //set input type 
    
    //disable all nav switch interrupts
    gpio_intr_disable(pin_nav_up);
    gpio_intr_disable(pin_nav_down);
    gpio_intr_disable(pin_nav_enter);

    vTaskNotifyGiveFromISR(local_switch->nav_switch_task_hdl, &xHighPriorityTaskWoken); //notify nav switch task nav switch input was detected
    portYIELD_FROM_ISR(xHighPriorityTaskWoken); //perform context switch if necessary
    
    
}

void IRAM_ATTR NavSwitch::enter_handler_ISR(void *arg)
{
    BaseType_t xHighPriorityTaskWoken = pdFALSE;
    NavSwitch *local_switch = (NavSwitch *)arg;
    local_switch->detected_input = nav_input_t::enter; //set input type
    
    //disable all nav switch interrupts
    gpio_intr_disable(pin_nav_up);
    gpio_intr_disable(pin_nav_down);
    gpio_intr_disable(pin_nav_enter);

    vTaskNotifyGiveFromISR(local_switch->nav_switch_task_hdl, &xHighPriorityTaskWoken); //notify nav switch task nav switch input was detected
    portYIELD_FROM_ISR(xHighPriorityTaskWoken); //perform context switch if necessary
    
    
}