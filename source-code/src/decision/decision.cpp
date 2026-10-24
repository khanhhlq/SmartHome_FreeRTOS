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
    bool haveDispatched = false;
    TickType_t lastDispatchedTick = 0;
    uint32_t lastDispatchedRevision = 0;
    SystemMode lastDispatchedMode = COMFORT;
    while (true) {
        // Notification from SensorTask, PIRTask or config update.
        // Fallback timeout also covers startup races when task handles were not ready.
        ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(DECISION_PERIOD_MS));
        if (xQueuePeek(sensorQueue, &sensor, 0) != pdTRUE ||
            !readControllerSettings(settings)) continue;

        decision.sensor = sensor;
        decision.settings = settings;
        decision.mode = evaluateMode(sensor, settings);
        const bool changed = !haveDispatched ||
            sensor.sampleTick != lastDispatchedTick ||
            settings.revision != lastDispatchedRevision ||
            decision.mode != lastDispatchedMode;
        if (!changed) continue;

        xQueueOverwrite(decisionQueue, &decision);
        haveDispatched = true;
        lastDispatchedTick = sensor.sampleTick;
        lastDispatchedRevision = settings.revision;
        lastDispatchedMode = decision.mode;
        if (controlTaskHandle) xTaskNotifyGive(controlTaskHandle);
    }
}
