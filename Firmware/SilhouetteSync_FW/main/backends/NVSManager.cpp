#include "NVSManager.hpp"

void NVSManager::initialize_NVS()
{
    esp_err_t err = nvs_flash_init(); 
    
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }
}

bool NVSManager::read_uint8_value(nvs_handle_t handle, const char* key, uint8_t* value)
{
    esp_err_t err;
    bool success = true;

    err = nvs_open("storage", NVS_READONLY, &handle);

    if (err != ESP_OK)
        success = false;
    else
    {
        err = nvs_get_u8(handle, key, value);

        if (err != ESP_OK)
            success = false;
    }

    nvs_close(handle);

    return success;
}

bool NVSManager::write_uint8_value(nvs_handle_t handle, const char* key, uint8_t value)
{
    esp_err_t err;
    bool success = true;

    // open storage
    err = nvs_open("storage", NVS_READWRITE, &handle);

    if (err != ESP_OK)
        success = false;
    else
    {
        // write value
        err = nvs_set_u8(handle, key, value);

        if (err != ESP_OK)
            success = false;
        else
        {
            // commit writes
            err = nvs_commit(handle);

            if (err != ESP_OK)
                success = false;
        }
    }

    nvs_close(handle);

    return success;
}