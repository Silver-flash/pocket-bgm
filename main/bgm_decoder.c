#include "bgm_decoder.h"
static const int steps[89] = {
    7,8,9,10,11,12,13,14,16,17,19,21,23,25,28,31,34,37,41,45,50,55,60,
    66,73,80,88,97,107,118,130,143,157,173,190,209,230,253,279,307,337,
    371,408,449,494,544,598,658,724,796,876,963,1060,1166,1282,1411,1552,
    1707,1878,2066,2272,2499,2749,3024,3327,3660,4026,4428,4871,5358,5894,
    6484,7132,7845,8630,9493,10442,11487,12635,13899,15289,16818,18500,
    20350,22385,24623,27086,29794,32767
};
static const int adjust[8] = {-1,-1,-1,-1,2,4,6,8};
bool bgm_decode(const uint8_t *data, size_t bytes, uint32_t samples,
                uint32_t cursor, int16_t *output, size_t count) {
    if (!data || !output || cursor > samples || count > samples-cursor) return false;
    while (count) {
        size_t block=cursor/505u, skip=cursor%505u;
        if (block >= bytes/256u) return false;
        const uint8_t *p=data+block*256u;
        int value=(int16_t)((unsigned)p[0]|((unsigned)p[1]<<8)), index=p[2];
        if (index>88 || p[3]!=0) return false;
        size_t take=505-skip;if(take>count)take=count;
        for (size_t n=0;n<skip+take;n++) {
            if (n) {
                unsigned code=(p[4+(n-1)/2] >> (((n-1)%2)*4)) & 15u;
                int step=steps[index];
                int delta=step>>3;
                if(code&1)delta+=step>>2;
                if(code&2)delta+=step>>1;
                if(code&4)delta+=step;
                value+=(code&8)?-delta:delta;
                if(value>32767)value=32767;
                if(value< -32768)value=-32768;
                index+=adjust[code&7];
                if(index<0)index=0;
                if(index>88)index=88;
            }
            if(n>=skip)*output++=(int16_t)value;
        }
        cursor+=(uint32_t)take;count-=take;
    }
    return true;
}
