#include "bgm_input.h"
#include "bgm_model.h"
#include <assert.h>
#include <stdio.h>
int main(void) {
    assert(bgm_logical_key(BSP_BTN_UP)==BSP_BTN_UP);
    assert(bgm_logical_key(BSP_BTN_DOWN)==BSP_BTN_OK);
    assert(bgm_logical_key(BSP_BTN_OK)==BSP_BTN_DOWN);
    bgm_model_t s; bgm_model_init(&s,11);
    bsp_btn_t middle=bgm_logical_key(BSP_BTN_DOWN);
    if(middle==BSP_BTN_OK) bgm_model_action(&s,BGM_SETTINGS);
    assert(s.settings && !s.playing);
    bsp_btn_t lower=bgm_logical_key(BSP_BTN_OK);
    if(lower==BSP_BTN_DOWN) bgm_model_action(&s,BGM_NEXT);
    assert(s.volume==20 && !s.playing);
    puts("BGM physical key mapping: PASS");
}
