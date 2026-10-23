#include "bgm_tracks.h"
#include <stddef.h>
extern const uint8_t track_0[] asm("_binary_track_00_adpcm_start");
extern const uint8_t track_1[] asm("_binary_track_01_adpcm_start");
extern const uint8_t track_2[] asm("_binary_track_02_adpcm_start");
const bgm_track_t bgm_tracks[] = {
    {"NEON", track_0, 96000u, 48896u},
    {"DRIFT", track_1, 96000u, 48896u},
    {"VICTORY", track_2, 96000u, 48896u},
};
const unsigned bgm_track_count = 3;
