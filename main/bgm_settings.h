#pragma once
#include "esp_err.h"
// Only volume is persisted. Playback always starts stopped.
esp_err_t bgm_settings_load(unsigned *volume);
esp_err_t bgm_settings_save(unsigned volume);
