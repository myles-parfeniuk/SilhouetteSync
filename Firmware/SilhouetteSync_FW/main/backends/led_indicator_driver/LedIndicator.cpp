#include "LedIndicator.hpp"

LedIndicator::LedIndicator(Device& d)
    : d(d)
    , leds({LED_COUNT, pin_leds_di})
    , active_anim(nullptr)
    , attempting_connection_anim(leds, static_cast<uint8_t>(AnimationPriorities::lan_connection_status), {true, true, false}, {2, 1, 1})
    , connected_anim(leds, static_cast<uint8_t>(AnimationPriorities::lan_connection_status), {0, 20, 0})
    , failed_connection_anim(leds, static_cast<uint8_t>(AnimationPriorities::lan_connection_status), {60, 0, 0})
    , calibration_anim(leds, static_cast<uint8_t>(AnimationPriorities::calibration_status), {false, false, true})
    , shutdown_anim(leds, static_cast<uint8_t>(AnimationPriorities::shutdown), {60, 0, 0}, 80, true)
    , boot_anim(leds, static_cast<uint8_t>(AnimationPriorities::boot), {0, 60, 0}, 80, true)
{
    d.lan_connection_status.follow(
            [this](LANConnectionStatus new_status)
            {
                switch (new_status)
                {
                case LANConnectionStatus::failed_connection:
                    ESP_LOGE(TAG, "FAILED");
                    add_animation_to_queue(&failed_connection_anim);
                    play_next_animation();
                    break;

                case LANConnectionStatus::attempting_connection:
                    ESP_LOGE(TAG, "ATTEMPTING");
                    add_animation_to_queue(&attempting_connection_anim);
                    play_next_animation();
                    break;

                case LANConnectionStatus::connected:
                    ESP_LOGE(TAG, "CONNECTED");
                    add_animation_to_queue(&connected_anim);
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
                    add_animation_to_queue(&calibration_anim);
                    play_next_animation();
                }
                else
                {
                    remove_animation_from_queue(AnimationPriorities::calibration_status);
                    play_next_animation();
                }
            });

    d.power_state.follow(
            [this](PowerStates new_state)
            {
                switch (new_state)
                {
                case PowerStates::boot:
                    add_animation_to_queue(&boot_anim);
                    play_next_animation();
                    break;

                case PowerStates::normal_operation:
                    remove_animation_from_queue(AnimationPriorities::boot);
                    play_next_animation();
                    break;

                case PowerStates::shutdown:
                    add_animation_to_queue(&shutdown_anim);
                    play_next_animation();
                    break;

                case PowerStates::low_power:
                    remove_animation_from_queue(AnimationPriorities::shutdown);
                    play_next_animation();
                    break;

                default:

                    break;
                }
            });
}

void LedIndicator::add_animation_to_queue(LedAnimation* new_animation)
{
    bool found = false;

    if (queued_anims.size() != 0)
    {
        for (int i = 0; i < queued_anims.size(); i++)
        {
            if (queued_anims.at(i)->get_priority() == new_animation->get_priority())
            {
                queued_anims.at(i) = new_animation;
                found = true;
            }
        }

        if (!found)
            queued_anims.push_back(new_animation);
    }
    else
    {
        queued_anims.push_back(new_animation);
    }
}

void LedIndicator::remove_animation_from_queue(AnimationPriorities priority)
{
    if (queued_anims.size() != 0)
    {
        for (int i = 0; i < queued_anims.size(); i++)
        {
            if (static_cast<AnimationPriorities>(queued_anims.at(i)->get_priority()) == priority)
                queued_anims.erase(queued_anims.begin() + i);
        }
    }
}

void LedIndicator::play_next_animation()
{
    uint8_t highest_priority = 0;
    uint8_t index = 0;
    if (queued_anims.size() != 0)
    {
        for (int i = 0; i < queued_anims.size(); i++)
        {
            if (queued_anims.at(i)->get_priority() > highest_priority)
            {
                highest_priority = queued_anims.at(i)->get_priority();
                index = i;
            }
        }

        if (queued_anims.at(index) != active_anim)
        {
            if (active_anim != nullptr)
                active_anim->stop();

            active_anim = queued_anims.at(index);
            active_anim->start();
        }
    }
}
