#include "bgm_gain.h"
#include <assert.h>
#include <stdio.h>
int main(void) {
    for (unsigned v=0;v<=500;v+=10) assert(bgm_codec_volume(v)==(v>100?100:v));
    for (unsigned v=100;v<=500;v+=100) {
        int previous=-32768;
        for (int x=-32768;x<=32767;x++) {
            bgm_gain_t g={v}; int16_t y=x;
            bgm_gain_process(&g,&y,1,v);
            assert(y>=previous); previous=y;
            if(v==100) assert(y==x);
            else { assert(y>=-32767 && y<=32767); if(x>0 && x<4000) assert(y==x*(int)v/100); }
            if(x==0) assert(y==0);
            if(x>0) { int16_t negative=-x; bgm_gain_process(&g,&negative,1,v); assert(negative==-y); }
        }
    }
    int16_t buffer[400]={0}; bgm_gain_t g={100};
    bgm_gain_process(&g,buffer,400,500); assert(g.percent==500);
    bgm_gain_process(&g,buffer,400,100); assert(g.percent==100);
    bgm_gain_process(&g,buffer,400,999); assert(g.percent==500);
    bgm_gain_process(&g,buffer,400,0); assert(g.percent==100);
    puts("BGM gain: PASS (full PCM range, monotonicity, symmetry, ramp and codec cap)");
}
