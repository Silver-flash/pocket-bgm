#include "bgm_gain.h"
unsigned bgm_codec_volume(unsigned volume) { return volume > 100 ? 100 : volume; }
void bgm_gain_process(bgm_gain_t *state, int16_t *pcm, size_t count, unsigned volume) {
    unsigned target = volume > 500 ? 500 : volume < 100 ? 100 : volume;
    if (state->percent < 100 || state->percent > 500) state->percent = 100;
    for (size_t i = 0; i < count; ++i) {
        // A full 1x to 5x transition takes 25 ms at 16 kHz.
        if (state->percent < target) ++state->percent;
        else if (state->percent > target) --state->percent;
        if (state->percent == 100) continue;
        int32_t value = (int32_t)pcm[i] * (int32_t)state->percent / 100;
        int32_t magnitude = value < 0 ? -value : value;
        unsigned knee_shift = state->percent - 100;
        if (knee_shift > 100) knee_shift = 100;
        int32_t knee = 32767 - (int32_t)knee_shift * 8191 / 100;
        int32_t headroom = 32767 - knee;
        if (magnitude > knee) {
            int32_t excess = magnitude - knee;
            magnitude = knee + excess * headroom / (excess + headroom);
        }
        pcm[i] = (int16_t)(value < 0 ? -magnitude : magnitude);
    }
}
