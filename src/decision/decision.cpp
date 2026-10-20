#include <Arduino.h>
#include "../core/config.h"
#include "../core/types.h"
#include "../core/app_state.h"
#include "decision.h"

// Khong phai FSM nho trang thai truoc: day la tap luat chon mode co uu tien.
SystemMode evaluateMode(const SensorData &data, const ControllerSettings &settings) {
    // WARNING bat bien 45C va uu tien cao nhat.
    if (data.temperature >= TEMP_WARNING) return WARNING;
    // PIR = 0 uu tien ENERGY_SAVING so voi cooling / lighting.
    if (!data.occupancy) return ENERGY_SAVING;
    if (data.temperature >= settings.coolingTempC) return COOLING;
    if (data.light < settings.darkLightPct) return LIGHTING;
    return COMFORT;
}

const char* modeName(SystemMode mode) {
    switch (mode) {
        case COMFORT: return "COMFORT";
        case COOLING: return "COOLING";
        case LIGHTING: return "LIGHTING";
        case ENERGY_SAVING: return "ENERGY_SAVING";
        case WARNING: return "WARNING";
        default: return "UNKNOWN";
    }
}

void DecisionTask(void *parameter) {
    SensorData sensor = {};
    ControllerSettings settings = {};
    DecisionData decision = {};
    while (true) {
        if (xQueuePeek(sensorQueue, &sensor, pdMS_TO_TICKS(100)) == pdTRUE &&
            readControllerSettings(settings)) {
            decision.sensor = sensor;
            decision.settings = settings;
            decision.mode = evaluateMode(sensor, settings);
            xQueueOverwrite(decisionQueue, &decision);
        }
        vTaskDelay(pdMS_TO_TICKS(DECISION_PERIOD_MS));
    }
}
