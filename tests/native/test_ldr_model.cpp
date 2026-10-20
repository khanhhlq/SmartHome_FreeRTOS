#include <cmath>
#include <cassert>
#include <iostream>
#include "sensors/light_math.h"
int main() {
    auto at500 = ldrFromLux(500.0f);
    assert(at500.adcRaw >= 998 && at500.adcRaw <= 1004);
    assert(std::fabs(at500.voltageV - 0.807f) < 0.006f);
    assert(std::fabs(at500.brightnessPct - 75.6f) < 0.3f);
    assert(std::fabs(ldrFromAdc(1001).lux - 500.0f) < 5.0f);
    auto dark = ldrFromLux(0);
    auto light = ldrFromLux(1000);
    assert(dark.adcRaw == 4095 && dark.brightnessPct == 0);
    assert(light.adcRaw < at500.adcRaw);
    assert(light.brightnessPct > at500.brightnessPct);
    for (float lux : {0.0f,1.0f,10.0f,30.0f,100.0f,500.0f,1000.0f,2000.0f}) {
        auto d = ldrFromLux(lux);
        assert(std::isfinite(d.voltageV) && std::isfinite(d.resistanceKOhm)
               && std::isfinite(d.lux) && std::isfinite(d.brightnessPct));
        assert(d.brightnessPct >= 0 && d.brightnessPct <= 100);
    }
    std::cout << "PASS: LDR 500 lux -> ADC " << at500.adcRaw
              << ", voltage " << at500.voltageV << " V, light "
              << at500.brightnessPct << "%\n";
}
