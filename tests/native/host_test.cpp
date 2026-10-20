#include <cassert>
#include <cmath>
#include <cstdio>
#include <string>
#include "Arduino.h"
#include "../../src/core/types.h"
#include "../../src/core/app_state.h"
#include "../../src/core/config.h"
#include "../../src/decision/decision.h"

SerialMock Serial;

int main() {
    assert(initRTOSObjects());
    ControllerSettings c={};
    assert(readControllerSettings(c));
    assert(c.coolingTempC == 30.0f && c.darkLightPct == 30.0f);
    assert(c.revision == 1);

    SensorData s={};
    s.temperature = 28;
    s.humidity = 50;
    s.light = 65;
    s.occupancy = true;
    assert(evaluateMode(s,c)==COMFORT);

    s.light=20;
    assert(evaluateMode(s,c)==LIGHTING);
    s.temperature=30;
    assert(evaluateMode(s,c)==COOLING);
    s.temperature=45;
    assert(evaluateMode(s,c)==WARNING);
    s.temperature=28;
    s.occupancy=false;
    assert(evaluateMode(s,c)==ENERGY_SAVING);
    s.temperature=45;
    assert(evaluateMode(s,c)==WARNING);  // highest priority even without PIR

    s.occupancy=true;
    s.temperature=32;
    s.light=50;
    assert(evaluateMode(s,c)==COOLING); // default 30 C
    ControllerSettings updated={};
    assert(updateControllerSettings(35,55,updated));
    assert(updated.revision == 2);
    assert(evaluateMode(s,updated)==LIGHTING); // 32<35 and 50<55
    s.light=60;
    assert(evaluateMode(s,updated)==COMFORT);
    s.temperature=35;
    assert(evaluateMode(s,updated)==COOLING); // inclusive boundary
    s.temperature=44.99;
    assert(evaluateMode(s,updated)==COOLING);
    s.temperature=45;
    assert(evaluateMode(s,updated)==WARNING); // always 45

    assert(!updateControllerSettings(45,30,updated));  // outside permitted bounds
    assert(!updateControllerSettings(30,80,updated));
    assert(!updateControllerSettings(NAN,30,updated));
    assert(!updateControllerSettings(30,INFINITY,updated));
    assert(readControllerSettings(c));
    assert(c.revision==2 && c.coolingTempC==35 && c.darkLightPct==55);
    assert(std::string(modeName(WARNING))=="WARNING");
    assert(LDR_ADC_INVERTED_TO_BRIGHTNESS);
    std::puts("PASS: default/updated thresholds, immutable WARNING, priority and input validation");
}
