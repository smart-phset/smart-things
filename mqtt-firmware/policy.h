#pragma once
#include <stdint.h>
#include <cmath>
#include <cstring>

inline float moisturePercent(int raw,int dry,int wet) {
    if(dry==wet) return NAN;
    float percent=100.0f*(raw-dry)/(wet-dry);
    return percent<0?0:percent>100?100:percent;
}
inline int actuatorIndex(const char* suffix) {
    if(strcmp(suffix,"fan")==0)return 0;
    if(strcmp(suffix,"light")==0)return 1;
    if(strcmp(suffix,"pump")==0)return 2;
    return -1;
}
inline bool freshCommand(int64_t expires,int64_t now) {
    return now>=1700000000 && expires>=now && expires<=now+15;
}
inline bool elapsed(uint32_t now,uint32_t before,uint32_t interval) {
    return static_cast<uint32_t>(now-before)>=interval;
}
