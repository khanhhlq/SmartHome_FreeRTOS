#pragma once
#include <Arduino.h>

struct SensorData {
    float temperature;
    float humidity;
    float light;
    bool occupancy;
    TickType_t sampleTick;
};

enum SystemMode {
    COMFORT,
    COOLING,
    LIGHTING,
    ENERGY_SAVING,
    WARNING
};

struct DecisionData {
    SystemMode mode;
};

enum InputSource {
    REAL_SENSOR,
    WEB_SENSOR
};
