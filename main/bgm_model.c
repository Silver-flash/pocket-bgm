#include "bgm_model.h"
void bgm_model_init(bgm_model_t *s, unsigned count) {
    *s = (bgm_model_t){ .count = count, .active = -1, .volume = 30, .loop = true };
}
void bgm_model_action(bgm_model_t *s, bgm_action_t action) {
    if (action == BGM_SETTINGS) { s->settings = !s->settings; return; }
    if (action == BGM_MODE) { s->loop = !s->loop; return; }
    if (action == BGM_PREV || action == BGM_NEXT) {
        if (s->settings) {
            if (action == BGM_PREV && s->volume < 500) s->volume += s->volume < 100 ? 10 : 100;
            if (action == BGM_NEXT && s->volume > 0) s->volume -= s->volume <= 100 ? 10 : 100;
        } else if (s->count) {
            s->selected = (s->selected + s->count + (action == BGM_PREV ? -1 : 1)) % s->count;
        }
        return;
    }
    if (action == BGM_TOGGLE) {
        if (s->settings) { s->loop = !s->loop; return; }
        if (!s->count) return;
        if (s->playing && s->active == (int)s->selected) {
            s->playing = false;
        } else {
            s->active = (int)s->selected;
            s->cursor = 0;
            s->playing = true;
        }
    }
}
void bgm_model_end(bgm_model_t *s) {
    s->cursor = 0;
    if (!s->loop) s->playing = false;
}
