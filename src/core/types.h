#pragma once
#include <Arduino.h>

// Gia tri REAL do SensorTask thu thap tu DHT22, LDR ADC1 va PIR.
struct SensorData {
    float temperature;
    float humidity;
    float light;          // Phan tram sang quy uoc (0: toi, 100: sang)
    bool occupancy;       // Trang thai logic tu PIR (su kien chuyen dong)
    TickType_t sampleTick;
    int ldrAdcRaw;
    float ldrVoltageV;    // Dien ap tinh theo mo hinh ly tuong 3.3 V
    float ldrResistanceKOhm;
    float ldrLux;         // Uoc tinh tu mo hinh LDR Wokwi
};

enum SystemMode {
    COMFORT, COOLING, LIGHTING, ENERGY_SAVING, WARNING
};

// Cac nguong duoc cap nhat NGUYEN CAP duoi mot FreeRTOS Mutex.
// Ngưỡng WARNING là hằng số trong config.h, không có trong settings.
struct ControllerSettings {
    float coolingTempC;
    float darkLightPct;
    uint32_t revision;
};

struct DecisionData {
    SystemMode mode;
    SensorData sensor;              // Snapshot cua cung mot lan danh gia
    ControllerSettings settings;    // Snapshot nguong de ControlTask khong lech
};
