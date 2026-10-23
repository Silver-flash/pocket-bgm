#include "bgm_settings.h"
#include "nvs.h"
#include "nvs_flash.h"
#include <stdbool.h>
#include <stdint.h>
static bool valid_volume(unsigned volume) {
    return volume <= 100 ? volume % 10 == 0 : volume <= 500 && volume % 100 == 0;
}
esp_err_t bgm_settings_load(unsigned *volume) {
    if (!volume) return ESP_ERR_INVALID_ARG;
    *volume = 30;
    // Never erase the shared NVS partition on an initialization failure.
    esp_err_t result = nvs_flash_init();
    if (result != ESP_OK) return result;
    nvs_handle_t handle;
    result = nvs_open("pocket_bgm", NVS_READONLY, &handle);
    if (result == ESP_ERR_NVS_NOT_FOUND) return ESP_OK;
    if (result != ESP_OK) return result;
    uint16_t saved = 30;
    result = nvs_get_u16(handle, "volume", &saved);
    nvs_close(handle);
    if (result == ESP_ERR_NVS_NOT_FOUND) return ESP_OK;
    if (result != ESP_OK) return result;
    if (!valid_volume(saved)) return ESP_ERR_INVALID_ARG;
    *volume = saved;
    return ESP_OK;
}
esp_err_t bgm_settings_save(unsigned volume) {
    if (!valid_volume(volume)) return ESP_ERR_INVALID_ARG;
    nvs_handle_t handle;
    esp_err_t result = nvs_open("pocket_bgm", NVS_READWRITE, &handle);
    if (result != ESP_OK) return result;
    result = nvs_set_u16(handle, "volume", (uint16_t)volume);
    if (result == ESP_OK) result = nvs_commit(handle);
    nvs_close(handle);
    return result;
}
