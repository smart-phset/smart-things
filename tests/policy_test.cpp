#include "../mqtt-firmware/policy.h"
#include <cassert>
int main() {
    assert(moisturePercent(3500,3500,1500)==0);
    assert(moisturePercent(1500,3500,1500)==100);
    assert(moisturePercent(2500,3500,1500)==50);
    assert(moisturePercent(0,3500,1500)==100);
    assert(moisturePercent(4095,3500,1500)==0);
    assert(moisturePercent(2500,1500,3500)==50);
    assert(std::isnan(moisturePercent(10,100,100)));
    const char* codes[]={"F0","F1","L0","L1","P0","P1"};
    for(int i=0;i<6;i++)assert(commandIndex(codes[i])==i/2);
    const char* invalid[]={"","P","F","L","P2","F-1","p1","P10"," P1","P1 ","X1","1P","ON"};
    for(const char* code:invalid)assert(commandIndex(code)==-1);
    assert(commandIndex(nullptr)==-1);
    char previous[65]={};
    assert(!rememberCommandId("",previous));
    assert(!rememberCommandId(nullptr,previous));
    char oversized[66];memset(oversized,'x',65);oversized[65]='\0';
    assert(!rememberCommandId(oversized,previous));
    assert(rememberCommandId("550e8400-e29b-41d4-a716-446655440000",previous));
    assert(!rememberCommandId("550e8400-e29b-41d4-a716-446655440000",previous));
    assert(rememberCommandId("550e8400-e29b-41d4-a716-446655440001",previous));
    // The same predicates run in setOutput()/loop(): repeated ON cannot reset timeout.
    assert(startsPumpTimer(2,true,false));
    assert(!startsPumpTimer(2,true,true));
    assert(!startsPumpTimer(2,false,true));
    assert(!startsPumpTimer(0,true,false));
    assert(!elapsed(10999,1000,10000));
    assert(elapsed(11000,1000,10000));
    assert(elapsed(4999,UINT32_MAX-5000,10000));
    assert(freshCommand(1800000010,1800000000));
    assert(!freshCommand(1799999999,1800000000));
    assert(!freshCommand(1800000020,1800000000));
    assert(!freshCommand(10,0));
    assert(elapsed(5,UINT32_MAX-4,10));
    assert(!elapsed(4,UINT32_MAX-4,10));
}
