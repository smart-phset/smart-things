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
    assert(actuatorIndex("fan")==0 && actuatorIndex("light")==1 && actuatorIndex("pump")==2);
    assert(actuatorIndex("water")==-1);
    assert(freshCommand(1800000010,1800000000));
    assert(!freshCommand(1799999999,1800000000));
    assert(!freshCommand(1800000020,1800000000));
    assert(!freshCommand(10,0));
    assert(elapsed(5,UINT32_MAX-4,10));
    assert(!elapsed(4,UINT32_MAX-4,10));
}
