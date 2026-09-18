#include <Arduino.h>

#include "../core/config.h"
#include "../core/types.h"
#include "../core/app_state.h"
#include "pir.h"

// ISR phải ngắn:
// chỉ ghi nhận thời điểm và phát semaphore.
void IRAM_ATTR pirISR() {
    BaseType_t taskWoken = pdFALSE;

    pirIsrTick = xTaskGetTickCountFromISR();

    xSemaphoreGiveFromISR(
        pirSemaphore,
        &taskWoken
    );

    if (taskWoken == pdTRUE)
        portYIELD_FROM_ISR();
}

void initPIRInterrupt() {
    currentOccupancy = digitalRead(PIR_PIN);

    attachInterrupt(
        digitalPinToInterrupt(PIR_PIN),
        pirISR,
        CHANGE
    );
}

void PIRTask(void *parameter) {
    while (true) {

        // Task ngủ cho đến khi ISR phát semaphore
        xSemaphoreTake(
            pirSemaphore,
            portMAX_DELAY
        );

        currentOccupancy = digitalRead(PIR_PIN);

        TickType_t now = xTaskGetTickCount();
        pirLatencyMs = (now - pirIsrTick) * portTICK_PERIOD_MS;

        Serial.print("[PIR] Occupancy = ");
        Serial.print(currentOccupancy ? "YES" : "NO");
        Serial.print(" | ISR->Task = ");
        Serial.print(pirLatencyMs);
        Serial.println(" ms");

        // Nếu đang dùng REAL sensor, cập nhật occupancy ngay
        if (inputSource == REAL_SENSOR) {
            SensorData sensor;

            if (xQueuePeek(sensorQueue, &sensor, 0) == pdTRUE) {
                sensor.occupancy = currentOccupancy;
                sensor.sampleTick = now;
                xQueueOverwrite(sensorQueue, &sensor);
            }
        }
    }
}
