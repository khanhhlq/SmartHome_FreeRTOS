#include "app_state.h"

// Tạo biến tham chiếu với Queue do FreeRTOS tạo ra
QueueHandle_t sensorQueue = nullptr;
QueueHandle_t decisionQueue = nullptr;
SemaphoreHandle_t pirSemaphore = nullptr;
SemaphoreHandle_t webMutex = nullptr;

volatile bool currentOccupancy = false;
volatile InputSource inputSource = REAL_SENSOR;
volatile TickType_t pirIsrTick = 0;

SensorData webData = {25.0f, 55.0f, 65.0f, true, 0};

volatile int fanPwm = 0;
volatile bool lightOn = false;
volatile bool alarmOn = false;

volatile uint32_t pirLatencyMs = 0;
volatile uint32_t responseTimeMs = 0;
volatile uint32_t controlExecMs = 0;
volatile uint32_t controlCycles = 0;
volatile uint32_t controlDeadlineMiss = 0;

bool initRTOSObjects() {
    // 1 nghĩa là chứa 1 item có kích thước bằng một SensorData. Lưu 1 item bởi vì chỉ cần lưu giá trị mới nhất hiện tại là gì
    sensorQueue   = xQueueCreate(1, sizeof(SensorData));
    decisionQueue = xQueueCreate(1, sizeof(DecisionData));
    pirSemaphore  = xSemaphoreCreateBinary();
    webMutex      = xSemaphoreCreateMutex();

    if (!sensorQueue || !decisionQueue || !pirSemaphore || !webMutex) {
        return false;
    }

    SensorData initial = {
        25.0f,
        55.0f,
        65.0f,
        currentOccupancy,
        xTaskGetTickCount()
    };

    xQueueOverwrite(sensorQueue, &initial);
    // decisionQueue will receive its first value from DecisionTask.

    return true;
}
