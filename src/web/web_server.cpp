#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

#include "../core/types.h"
#include "../core/app_state.h"
#include "../decision/decision.h"
#include "web_server.h"
#include "web_page.h"

static WebServer server(80);

static void handleRoot() {
    server.send_P(200, "text/html", WEB_PAGE);
}

static void handleWebSensor() {
    if (!server.hasArg("temperature") ||
        !server.hasArg("humidity") ||
        !server.hasArg("light") ||
        !server.hasArg("occupancy")) {

        server.send(
            400,
            "application/json",
            "{\"ok\":false}"
        );
        return;
    }

    SensorData data;
    data.temperature = server.arg("temperature").toFloat();
    data.humidity    = server.arg("humidity").toFloat();
    data.light       = server.arg("light").toFloat();
    data.occupancy   = server.arg("occupancy").toInt() == 1;
    data.sampleTick  = xTaskGetTickCount();

    // webData là vùng dùng chung -> bảo vệ bằng Mutex
    if (xSemaphoreTake(webMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
        webData = data;
        xSemaphoreGive(webMutex);
    }

    inputSource = WEB_SENSOR;

    // Gửi ngay để không phải đợi chu kỳ SensorTask tiếp theo
    xQueueOverwrite(sensorQueue, &data);

    server.send(
        200,
        "application/json",
        "{\"ok\":true}"
    );
}

static void handleRealSensor() {
    inputSource = REAL_SENSOR;

    server.send(
        200,
        "application/json",
        "{\"ok\":true}"
    );
}

static void handleState() {
    SensorData sensor = {};
    DecisionData decision = {ENERGY_SAVING};

    xQueuePeek(sensorQueue, &sensor, 0);
    xQueuePeek(decisionQueue, &decision, 0);

    int fanPercent = (int)((fanPwm / 255.0f) * 100.0f);

    float deadlineRate = 100.0f;
    if (controlCycles > 0) {
        deadlineRate =
            100.0f *
            (controlCycles - controlDeadlineMiss) /
            controlCycles;
    }

    String json = "{";
    json += "\"source\":\"" + String(sourceName()) + "\",";
    json += "\"temperature\":" + String(sensor.temperature, 1) + ",";
    json += "\"humidity\":" + String(sensor.humidity, 1) + ",";
    json += "\"light\":" + String(sensor.light, 1) + ",";
    json += "\"occupancy\":" + String(sensor.occupancy ? "true" : "false") + ",";
    json += "\"mode\":\"" + String(modeName(decision.mode)) + "\",";
    json += "\"fan\":" + String(fanPercent) + ",";
    json += "\"lightOn\":" + String(lightOn ? "true" : "false") + ",";
    json += "\"alarmOn\":" + String(alarmOn ? "true" : "false") + ",";
    json += "\"pirLatency\":" + String(pirLatencyMs) + ",";
    json += "\"responseTime\":" + String(responseTimeMs) + ",";
    json += "\"controlExec\":" + String(controlExecMs) + ",";
    json += "\"deadlineRate\":" + String(deadlineRate, 1);
    json += "}";

    server.sendHeader("Cache-Control", "no-store");
    server.send(200, "application/json", json);
}

void WebTask(void *parameter) {
    WiFi.begin("Wokwi-GUEST", "", 6);

    while (WiFi.status() != WL_CONNECTED) {
        vTaskDelay(pdMS_TO_TICKS(250));
    }

    Serial.println("[WEB] WiFi connected");

    server.on("/", HTTP_GET, handleRoot);
    server.on("/api/state", HTTP_GET, handleState);
    server.on("/api/sensors", HTTP_POST, handleWebSensor);
    server.on("/api/real", HTTP_POST, handleRealSensor);

    server.begin();

    Serial.println("[WEB] HTTP server started");
    Serial.println("[WEB] Open http://localhost:8180");

    while (true) {
        server.handleClient();
        vTaskDelay(pdMS_TO_TICKS(2));
    }
}
