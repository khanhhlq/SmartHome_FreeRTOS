#include "app_state.h"
#include "config.h"
#include <math.h>

QueueHandle_t sensorQueue = nullptr;
QueueHandle_t decisionQueue = nullptr;
SemaphoreHandle_t pirSemaphore = nullptr;
SemaphoreHandle_t configMutex = nullptr;
TaskHandle_t decisionTaskHandle = nullptr;
TaskHandle_t controlTaskHandle = nullptr;

volatile bool currentOccupancy = false;
volatile TickType_t pirIsrTick = 0;
ControllerSettings controllerSettings = {
    TEMP_COOLING_DEFAULT, LIGHT_DARK_DEFAULT, 1
};

volatile int fanPwm = 0;
volatile bool lightOn = false;
volatile bool alarmOn = false;
volatile uint32_t pirLatencyMs = 0;
volatile uint32_t responseTimeMs = 0;
volatile uint32_t controlExecMs = 0;
volatile uint32_t controlExecUs = 0;
volatile uint32_t controlCycles = 0;
volatile uint32_t controlDeadlineMiss = 0;
volatile uint32_t maxResponseTimeMs = 0;

bool initRTOSObjects() {
    sensorQueue   = xQueueCreate(1, sizeof(SensorData));
    decisionQueue = xQueueCreate(1, sizeof(DecisionData));
    pirSemaphore  = xSemaphoreCreateBinary();
    configMutex   = xSemaphoreCreateMutex();
    return sensorQueue && decisionQueue && pirSemaphore && configMutex;
}

bool readControllerSettings(ControllerSettings &out) {
    if (!configMutex || xSemaphoreTake(configMutex, pdMS_TO_TICKS(50)) != pdTRUE)
        return false;
    out = controllerSettings;    // lay ban sao nhat quan
    xSemaphoreGive(configMutex);
    return true;
}

bool updateControllerSettings(float coolingTempC, float darkLightPct,
                              ControllerSettings &out) {
    // Nguong 45C WARNING khong duoc cho phep thay doi qua Web.
    if (!isfinite(coolingTempC) || !isfinite(darkLightPct) ||
        coolingTempC < TEMP_COOLING_MIN || coolingTempC > TEMP_COOLING_MAX ||
        darkLightPct < LIGHT_DARK_MIN || darkLightPct > LIGHT_DARK_MAX)
        return false;
    if (!configMutex || xSemaphoreTake(configMutex, pdMS_TO_TICKS(50)) != pdTRUE)
        return false;
    controllerSettings.coolingTempC = coolingTempC;
    controllerSettings.darkLightPct = darkLightPct;
    controllerSettings.revision++; // revision giup Web biet config da ap dung
    out = controllerSettings;
    xSemaphoreGive(configMutex);
    // Notify DecisionTask only after releasing configMutex.
    if (decisionTaskHandle) xTaskNotifyGive(decisionTaskHandle);
    return true;
}
