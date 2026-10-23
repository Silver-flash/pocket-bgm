#include "bgm_model.h"
#include <assert.h>
#include <stdio.h>
int main(void) {
    bgm_model_t s;
    bgm_model_init(&s, 4);
    assert(!s.playing && s.active == -1 && s.volume == 30);
    bgm_model_action(&s, BGM_PREV); assert(s.selected == 3);
    bgm_model_action(&s, BGM_NEXT); assert(s.selected == 0);
    bgm_model_action(&s, BGM_TOGGLE); assert(s.playing && s.active == 0);
    s.cursor=123;
    bgm_model_action(&s, BGM_NEXT); assert(s.playing && s.active == 0 && s.cursor==123);
    bgm_model_action(&s, BGM_TOGGLE); assert(s.playing && s.active == 1 && s.cursor==0);
    bgm_model_action(&s, BGM_TOGGLE); assert(!s.playing);
    bgm_model_action(&s, BGM_SETTINGS);
    for(int i=0;i<100;i++) bgm_model_action(&s,BGM_PREV);
    assert(s.volume==500 && s.selected==1);
    for(int i=0;i<100;i++) bgm_model_action(&s,BGM_NEXT);
    assert(s.volume==0);
    const unsigned levels[] = {10,20,30,40,50,60,70,80,90,100,200,300,400,500};
    for (unsigned i=0;i<14;i++) { bgm_model_action(&s,BGM_PREV); assert(s.volume==levels[i]); }
    for (int i=12;i>=0;i--) { bgm_model_action(&s,BGM_NEXT); assert(s.volume==levels[i]); }
    bgm_model_action(&s,BGM_NEXT); assert(s.volume==0);
    bgm_model_action(&s,BGM_TOGGLE); assert(!s.loop && !s.playing);
    bgm_model_action(&s,BGM_SETTINGS);
    bgm_model_action(&s,BGM_TOGGLE); bgm_model_end(&s); assert(!s.playing);
    bgm_model_action(&s,BGM_MODE);
    bgm_model_action(&s,BGM_TOGGLE); s.cursor=100; bgm_model_end(&s);
    assert(s.playing && s.cursor==0);
    bgm_model_init(&s,0); bgm_model_action(&s,BGM_TOGGLE); assert(!s.playing);
    puts("BGM model: PASS");
}
