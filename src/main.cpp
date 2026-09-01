#include <Arduino.h> // Import thư viện của Arduino

// Include các module
#include "core/config.h" 
#include "core/app_state.h"
#include "sensors/sensors.h"
#include "decision/decision.h"
#include "control/control.h"
#include "pir/pir.h"
#include "display/display.h"
#include "web/web_server.h"

// =========================
// main.cpp chỉ làm 3 việc:
// 1. Khởi tạo phần cứng
// 2. Tạo các object FreeRTOS
// 3. Tạo các task
// =========================

// Hàm chạy một lần khi ESP32 khởi động
void setup() {
    Serial.begin(115200); // Mở giao tiếp Serial với baud rate là 115200 bits/s

    Serial.println();
    Serial.println("==============================");
    Serial.println(" SMART HOME - FREERTOS + FSM ");
    Serial.println("==============================");

    // 1. Khởi tạo phần cứng
    initSensors();
    initControl();
    initDisplay();

        /// TRUE OR FALSE
    if (!initRTOSObjects()) { 
        Serial.println("[ERROR] Cannot create FreeRTOS objects");
        // Vòng lặp vô hạn để dừng khởi tạo. Nếu khong tạo được Queue , Semaphore và Mutex thì hệ thống không thể hoạt động đúng
        // Thì tốt nhất nên dừng hệ thống bằng vòng lặp ở dưới đây
        while (true) {
            delay(1000);
        }
    }

    // Interrupt cần pirSemaphore đã tồn tại
    initPIRInterrupt();

    // 3. Tạo các task
    xTaskCreate(
        PIRTask,
        "PIRTask",
        2048,
        nullptr,
        4,
        nullptr
    );

    xTaskCreate(
        ControlTask,
        "ControlTask",
        4096,
        nullptr,
        3,
        nullptr
    );

    xTaskCreate(
        SensorTask,
        "SensorTask",
        4096,
        nullptr,
        2,
        nullptr
    );

    xTaskCreate(
        DecisionTask,
        "DecisionTask",
        3072,
        nullptr,
        2,
        nullptr
    );

    xTaskCreate(
        DisplayTask,
        "DisplayTask",
        3072,
        nullptr,
        1,
        nullptr
    );

    xTaskCreate(
        WebTask,
        "WebTask",
        6144,
        nullptr,
        1,
        nullptr
    );

    Serial.println("[SYSTEM] Tasks created");
}

// Arduino loop không còn xử lý logic chính.
// Hệ thống được vận hành bởi các FreeRTOS task.
void loop() {
    // FreeRTOS làm việc theo tick không nhất thiết trực tiếp theo ms, ta sài macro để chuyển ms to tick 
    vTaskDelay(pdMS_TO_TICKS(1000));
}
