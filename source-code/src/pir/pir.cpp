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

        // Một dòng cho một sự kiện: tránh các Serial.print rời rạc.
        Serial.printf("[PIR]  GPIO16=%d  ISR->PIRTask=%lu ms\r\n",
                      currentOccupancy ? 1 : 0, (unsigned long)pirLatencyMs);

        // PIR event: cap nhat ngay trang thai trong sensorQueue (1 phan tu).
        // ISR chi bao semaphore; thao tac Queue thuc hien tai PIRTask.
        SensorData sensor = {};
        if (xQueuePeek(sensorQueue, &sensor, 0) == pdTRUE) {
            sensor.occupancy = currentOccupancy;
            sensor.sampleTick = now;
            xQueueOverwrite(sensorQueue, &sensor);
            if (decisionTaskHandle) xTaskNotifyGive(decisionTaskHandle);
        }
    }
}
