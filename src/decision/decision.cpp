#include <Arduino.h>

#include "../core/config.h"
#include "../core/types.h"
#include "../core/app_state.h"
#include "decision.h"

// =====================================================
// FSM / RULE-BASED CONTROL
// =====================================================
SystemMode evaluateMode(const SensorData &data) {

    // 1. Nguy hiểm -> ưu tiên cao nhất
    if (data.temperature >= TEMP_WARNING)
        return WARNING;

    // 2. Không có người -> tiết kiệm năng lượng
    if (!data.occupancy)
        return ENERGY_SAVING;

    // 3. Có người và nóng -> làm mát
    if (data.temperature >= TEMP_COOLING)
        return COOLING;

    // 4. Có người, nhiệt độ bình thường nhưng tối -> bật đèn
    if (data.light < LIGHT_DARK)
        return LIGHTING;

    // 5. Còn lại -> thoải mái
    return COMFORT;
}

const char* modeName(SystemMode mode) {
    switch (mode) {
        case COMFORT:       return "COMFORT";
        case COOLING:       return "COOLING";
        case LIGHTING:      return "LIGHTING";
        case ENERGY_SAVING: return "ENERGY SAVING";
        case WARNING:       return "WARNING";
        default:            return "UNKNOWN";
    }
}

const char* sourceName() {
    return inputSource == WEB_SENSOR ? "WEB" : "REAL";
}

void DecisionTask(void *parameter) {
    SensorData sensor;
    DecisionData decision;

    while (true) {
        if (xQueuePeek(sensorQueue, &sensor, pdMS_TO_TICKS(100)) == pdTRUE) {
            decision.mode = evaluateMode(sensor);
            xQueueOverwrite(decisionQueue, &decision);
        }

        vTaskDelay(pdMS_TO_TICKS(DECISION_PERIOD_MS));
    }
}
