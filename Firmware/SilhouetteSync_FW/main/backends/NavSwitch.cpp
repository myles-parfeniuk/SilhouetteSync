#include "NavSwitch.hpp"

bool NavSwitch::isr_service_installed = false; 

NavSwitch::NavSwitch(Device &d):
d(d),
switch_code(0),
debounced(false)
{
    gpio_config_t switch_config; 
    esp_timer_create_args_t debounce_timer_conf;

    //configure GPIO pins
    switch_config.pin_bit_mask = (1U << pin_nav_up) | (1U << pin_nav_down); //select gpio pins to configure
    switch_config.intr_type = GPIO_INTR_NEGEDGE; //falling edge, active low inputs
    switch_config.pull_down_en = GPIO_PULLDOWN_DISABLE; //disable internal pullup
    switch_config.pull_up_en = GPIO_PULLUP_DISABLE; //disable internal pulldown
    switch_config.mode = GPIO_MODE_INPUT; //set gpio as input
    gpio_config(&switch_config);



    if(!isr_service_installed)
    {
        isr_service_installed = true; 
        gpio_install_isr_service(0); //install GPIO ISR handler service 
    }
    
    //add isr handlers
    gpio_isr_handler_add(pin_nav_up, nav_up_ISR, this);
    gpio_isr_handler_add(pin_nav_down, nav_down_ISR, this);

    //configure debounce timer
    debounce_timer_conf.arg = (void *)this;
    debounce_timer_conf.callback = &debounce_cb;
    debounce_timer_conf.name = "debounce_timer";
    debounce_timer_conf.dispatch_method = ESP_TIMER_TASK; 
    ESP_ERROR_CHECK(esp_timer_create(&debounce_timer_conf, &debounce_timer_hdl));

    xTaskCreate(&nav_switch_task_trampoline, "nav_switch_task", 8192, this, 5, &nav_switch_task_hdl);
}

void NavSwitch::nav_switch_task_trampoline(void *nav_switch)
{
    NavSwitch *active_switch = (NavSwitch *)nav_switch;
    active_switch->nav_switch_task();
}

void NavSwitch::nav_switch_task()
{

    while(1)
    {
        vTaskSuspend(NULL); //self suspend task
        //wait for debounce
        while(!debounced)
        {
            vTaskDelay(10/portTICK_PERIOD_MS); 
        }

        //disable interrupts on both inputs
        gpio_intr_disable(pin_nav_up); 
        gpio_intr_disable(pin_nav_down); 

        //check for press type, if button is not released (long press) then check for release event
        if(!press_check())
            released_check();

        
        //ensure both inputs are released, no events are generated here
        while(!gpio_get_level(pin_nav_down) || !gpio_get_level(pin_nav_up))
        {
            vTaskDelay(15/portTICK_PERIOD_MS); 
        }

        vTaskDelay(15/portTICK_PERIOD_MS); //debounce release

        //clear switch code
        switch_code = 0; 

        //re-enable interrupts on both inputs 
        gpio_intr_enable(pin_nav_up);
        gpio_intr_enable(pin_nav_down);   

        //reset debounce
        debounced = false; 
    }

}

bool NavSwitch::press_check()
{
    int64_t time_stamp = esp_timer_get_time();
    bool press_check = false;
    bool released = false; 


    while(!press_check)
    {
        switch(switch_code)
        {
            case BOTH_PRESSED:
                if(gpio_get_level(pin_nav_down) || gpio_get_level(pin_nav_up))
                {
                    press_check = true;
                    released = true;
                    generate_quick_press_evt(); 
                }
            break;

            case UP_PRESSED:
                if(gpio_get_level(pin_nav_up))
                {
                    press_check = true;
                    released = true;
                    generate_quick_press_evt(); 
                }
            break;

            case DOWN_PRESSED:
                if(gpio_get_level(pin_nav_down))
                {
                    press_check = true;
                    released = true;
                    generate_quick_press_evt(); 
                }
            break;

            default:
            break;
        }

        if((esp_timer_get_time() - time_stamp) > long_press_time_us)
        {
            //generate long press event and exit long-press check, progress to release check
            press_check = true; 
            generate_long_press_evt(); //generate long press event
        }

        vTaskDelay(15/portTICK_PERIOD_MS); 
    }

    return released; 

}

void NavSwitch::released_check()
{
    bool released_check = false;
    int64_t time_stamp = esp_timer_get_time(); 

    //release check: generate held events until release event
    while(!released_check)
    {
        switch(switch_code)
        {
            case BOTH_PRESSED:
                if(gpio_get_level(pin_nav_down) || gpio_get_level(pin_nav_up))
                {
                    released_check = true;
                    generate_released_evt();
                }
            break;

            case UP_PRESSED:
                if(gpio_get_level(pin_nav_up))
                {
                    released_check = true;
                    generate_released_evt();
                }
            break;

            case DOWN_PRESSED:
                if(gpio_get_level(pin_nav_down))
                {
                    released_check = true;
                    generate_released_evt();
                }
            break;

            default:
            break;
        }

        //if held event generation time exceeded
        if((esp_timer_get_time() - time_stamp) > held_time_us)
        {
            //reset time stamp generate held event
            time_stamp = esp_timer_get_time(); 
            generate_held_evt();
                        
        }

        vTaskDelay(15/portTICK_PERIOD_MS);
    }

}

void NavSwitch::generate_long_press_evt()
{
        switch(switch_code)
        {
            case BOTH_PRESSED:
                d.nav_switch.set(NavSwitchEvent::enter_long_press);
            break;

            case UP_PRESSED:
                d.nav_switch.set(NavSwitchEvent::up_long_press);
            break;

            case DOWN_PRESSED:
                d.nav_switch.set(NavSwitchEvent::down_long_press);
            break;

            default:

            break;

        }
}

void NavSwitch::generate_quick_press_evt()
{
        switch(switch_code)
        {
            case BOTH_PRESSED:
                d.nav_switch.set(NavSwitchEvent::enter_quick_press);
            break;

            case UP_PRESSED:
                d.nav_switch.set(NavSwitchEvent::up_quick_press);
            break;

            case DOWN_PRESSED:
                d.nav_switch.set(NavSwitchEvent::down_quick_press);
            break;

            default:

            break;

        }
}

void NavSwitch::generate_held_evt()
{
        switch(switch_code)
        {
            case BOTH_PRESSED:
                d.nav_switch.set(NavSwitchEvent::enter_held);
            break;

            case UP_PRESSED:
                d.nav_switch.set(NavSwitchEvent::up_held);
            break;

            case DOWN_PRESSED:
                d.nav_switch.set(NavSwitchEvent::down_held);
            break;

            default:

            break;

        }
}

void NavSwitch::generate_released_evt()
{
    d.nav_switch.set(NavSwitchEvent::released);
}


void NavSwitch::debounce_cb(void *arg)
{
    NavSwitch *active_switch = (NavSwitch *)arg;
    active_switch->debounced = true;  
}

void IRAM_ATTR NavSwitch::nav_up_ISR(void *arg)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    NavSwitch *active_switch = (NavSwitch *)arg;

    gpio_intr_disable(pin_nav_up); //disable interrupts on up pin

    //set switch code
    active_switch->switch_code |= UP_PRESSED; 

    //start debounce timer
    esp_timer_stop(active_switch->debounce_timer_hdl); //stop to reset if timer already running
    esp_timer_start_once(active_switch->debounce_timer_hdl, debounce_time_us);
    


    xHigherPriorityTaskWoken = xTaskResumeFromISR(active_switch->nav_switch_task_hdl); //resume nav switch task
    //only do context switch if task is suspended
    if(xHigherPriorityTaskWoken == pdTRUE)
    {
        //Perform a context switch so this interrupt returns directly to the unblocked task
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

void IRAM_ATTR NavSwitch::nav_down_ISR(void *arg)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    NavSwitch *active_switch = (NavSwitch *)arg;

    gpio_intr_disable(pin_nav_down); //disable interrupts on down pin
    
    //set switch code
    active_switch->switch_code |= DOWN_PRESSED; 

    //start debounce timer
    esp_timer_stop(active_switch->debounce_timer_hdl); //stop to reset if timer already running
    esp_timer_start_once(active_switch->debounce_timer_hdl, debounce_time_us); 
    

    xHigherPriorityTaskWoken = xTaskResumeFromISR(active_switch->nav_switch_task_hdl); //resume nav switch task
    //only do context switch if task is suspended
    if(xHigherPriorityTaskWoken == pdTRUE)
    {
        //Perform a context switch so this interrupt returns directly to the unblocked task
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }

}