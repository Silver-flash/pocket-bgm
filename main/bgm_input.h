#pragma once
#include "bsp_button.h"
// Physical positions confirmed by the user: upper=UP, middle=DOWN, lower=OK.
static inline bsp_btn_t bgm_logical_key(bsp_btn_t raw) {
    return raw == BSP_BTN_DOWN ? BSP_BTN_OK : raw == BSP_BTN_OK ? BSP_BTN_DOWN : raw;
}
