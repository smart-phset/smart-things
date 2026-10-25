#pragma once
#include <stdint.h>
#include <cmath>
#include <cstring>

inline float moisturePercent(int raw,int dry,int wet) {
    if(dry==wet) return NAN;
    float percent=100.0f*(raw-dry)/(wet-dry);
    return percent<0?0:percent>100?100:percent;
}
// Only exact, explicit assignments are valid; never interpret toggle commands.
inline int commandIndex(const char* code) {
    if(!code || strlen(code)!=2 || (code[1]!='0' && code[1]!='1'))return -1;
    switch(code[0]) {case 'F':return 0;case 'L':return 1;case 'P':return 2;default:return -1;}
}
inline bool validCommandId(const char* id) {
    return id && strlen(id)>0 && strlen(id)<=64;
}
inline bool rememberCommandId(const char* id,char (&previous)[65]) {
    if(!validCommandId(id) || strcmp(previous,id)==0)return false;
    strcpy(previous,id);
    return true;
}
inline bool startsPumpTimer(int index,bool on,bool wasOn) {
    return index==2 && on && !wasOn;
}
inline bool freshCommand(int64_t expires,int64_t now) {
    return now>=1700000000 && expires>=now && expires<=now+15;
}
inline bool elapsed(uint32_t now,uint32_t before,uint32_t interval) {
    return static_cast<uint32_t>(now-before)>=interval;
}
