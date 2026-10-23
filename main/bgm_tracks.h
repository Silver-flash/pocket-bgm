#pragma once
#include <stdint.h>
typedef struct {
    const char *title;
    const uint8_t *pcm;
    uint32_t samples;
    uint32_t bytes;
} bgm_track_t;
extern const bgm_track_t bgm_tracks[];
extern const unsigned bgm_track_count;
