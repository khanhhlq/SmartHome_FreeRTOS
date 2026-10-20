#include <Arduino.h>

#include "core/config.h"
#include "core/app_state.h"
#include "sensors/sensors.h"
#include "decision/decision.h"
#include "control/control.h"
#include "pir/pir.h"
#include "display/display.h"
#include "web/web_server.h"

// setup(): khoi tao phan cung, object FreeRTOS, roi tao 6 task.
// Cac task va uu tien duoc giu nguyen so voi source goc cua nhom.
void setup() {
    Serial.begin(115200);
    Serial.println();
    Serial.println("=====================================");
    Serial.println(" ESP32-S3 SMART HOME - REAL + CONFIG ");
    Serial.println("=====================================");

    initSensors();
    initControl();
    initDisplay();
    currentOccupancy = digitalRead(PIR_PIN);

    if (!initRTOSObjects()) {
        Serial.println("[ERROR] Cannot create RTOS Queue/Semaphore/Mutex");
        while (true) delay(1000);
    }
    initPIRInterrupt();  // pirSemaphore da ton tai truoc khi ISR bat dau

    bool created = true;
    created &= (xTaskCreate(PIRTask, "PIRTask", 2048, nullptr, 4, nullptr) == pdPASS);
    created &= (xTaskCreate(ControlTask, "ControlTask", 4096, nullptr, 3, nullptr) == pdPASS);
    created &= (xTaskCreate(SensorTask, "SensorTask", 4096, nullptr, 2, nullptr) == pdPASS);
    created &= (xTaskCreate(DecisionTask, "DecisionTask", 3072, nullptr, 2, nullptr) == pdPASS);
    created &= (xTaskCreate(DisplayTask, "DisplayTask", 3072, nullptr, 1, nullptr) == pdPASS);
    created &= (xTaskCreate(WebTask, "WebTask", 6144, nullptr, 1, nullptr) == pdPASS);
    if (!created) {
        Serial.println("[ERROR] One or more FreeRTOS tasks could not be created");
        while (true) delay(1000);
    }
    Serial.println("[SYSTEM] 6 FreeRTOS tasks created; sensor source: REAL only");
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000));
}
