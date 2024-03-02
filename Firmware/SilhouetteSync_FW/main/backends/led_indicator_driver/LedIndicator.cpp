#include "LedIndicator.hpp"

LedIndicator::LedIndicator(Device& d)
    : d(d)
    , leds({LED_COUNT, pin_leds_di})
    , active_animation(nullptr)
    , attempting_connection_animation(leds, 0, {true, true, false}, {2, 1, 1})
    , connected_animation(leds, 0, {0, 30, 0})
    , failed_connection_animation(leds, 0, {120, 0, 0})
    , calibration_animation(leds, 1, {false, false, true})
{
    d.lan_connection_status.follow(
            [this](LANConnectionStatus new_status)
            {
                switch (new_status)
                {
                case LANConnectionStatus::failed_connection:
                    ESP_LOGE(TAG, "FAILED");
                    add_animation_to_queue(&failed_connection_animation);
                    play_next_animation();
                    break;

                case LANConnectionStatus::attempting_connection:
                    ESP_LOGE(TAG, "ATTEMPTING");
                    add_animation_to_queue(&attempting_connection_animation);
                    play_next_animation();
                    break;

                case LANConnectionStatus::connected:
                    ESP_LOGE(TAG, "CONNECTED");
                    add_animation_to_queue(&connected_animation);
                    play_next_animation();
                    break;

                default:

                    break;
                }
            });

    d.imu.state.follow(
            [this](IMUState new_state)
            {
                if (new_state == IMUState::calibrate)
                {
                    add_animation_to_queue(&calibration_animation);
                    play_next_animation();
                }
                else
                {
                    remove_animation_from_queue(1);
                    play_next_animation();
                }
            });
}

void LedIndicator::add_animation_to_queue(LedAnimation* new_animation)
{
    bool found = false;

    if (queued_animations.size() != 0)
    {
        for (int i = 0; i < queued_animations.size(); i++)
        {
            if (queued_animations.at(i)->get_priority() == new_animation->get_priority())
            {
                queued_animations.at(i) = new_animation;
                found = true;
            }
        }

        if (!found)
            queued_animations.push_back(new_animation);
    }
    else
    {
        queued_animations.push_back(new_animation);
    }
}

void LedIndicator::remove_animation_from_queue(uint8_t priority)
{
    if (queued_animations.size() != 0)
    {
        for (int i = 0; i < queued_animations.size(); i++)
        {
            if (queued_animations.at(i)->get_priority() == priority)
                queued_animations.erase(queued_animations.begin() + i);
        }
    }
}

void LedIndicator::play_next_animation()
{
    uint8_t highest_priority = 0;
    uint8_t index = 0;
    if (queued_animations.size() != 0)
    {
        for (int i = 0; i < queued_animations.size(); i++)
        {
            if (queued_animations.at(i)->get_priority() > highest_priority)
            {
                highest_priority = queued_animations.at(i)->get_priority();
                index = i;
            }
        }

        if (queued_animations.at(index) != active_animation)
        {
            if (active_animation != nullptr)
                active_animation->stop();

            active_animation = queued_animations.at(index);
            active_animation->start();
        }
    }
}
