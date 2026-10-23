#pragma once
#include <stddef.h>
#include <stdint.h>
typedef struct { unsigned percent; } bgm_gain_t;
unsigned bgm_codec_volume(unsigned volume);
void bgm_gain_process(bgm_gain_t *state, int16_t *pcm, size_t count, unsigned volume);
