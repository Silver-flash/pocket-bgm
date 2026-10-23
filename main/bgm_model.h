#pragma once
#include <stdbool.h>
#include <stdint.h>
typedef enum { BGM_PREV, BGM_NEXT, BGM_TOGGLE, BGM_SETTINGS, BGM_MODE } bgm_action_t;
typedef struct {
    unsigned selected, count;
    int active;
    unsigned volume;
    bool loop, settings, playing;
    uint32_t cursor;
} bgm_model_t;
void bgm_model_init(bgm_model_t *s, unsigned count);
void bgm_model_action(bgm_model_t *s, bgm_action_t action);
void bgm_model_end(bgm_model_t *s);
