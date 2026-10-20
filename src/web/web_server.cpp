#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <math.h>
#include <stdlib.h>

#include "../core/config.h"
#include "../core/types.h"
#include "../core/app_state.h"
#include "../decision/decision.h"
#include "web_server.h"
#include "web_page.h"

// HTTP server chay tren ESP32-S3; Web chi giam sat REAL va cau hinh FSM.
static WebServer server(80);

static void preventCache() {
    server.sendHeader("Cache-Control", "no-store");
}

static void sendError(int code, const char* reason) {
    preventCache();
    server.send(code, "application/json", String("{\"ok\":false,\"error\":\"") +
                          reason + "\"}");
}

static void handleRoot() {
    preventCache();
    server.send_P(200, "text/html", WEB_PAGE);
}

// POST /api/config: atomic update 2 configurable thresholds.
// WARNING >=45 C, occupancy rule and FAN_MAX remain constants in firmware.
static bool parseFloatField(const char* field, float &out) {
    if (!server.hasArg(field)) return false;
    const String text = server.arg(field);
    const char* c = text.c_str();
    char* end = nullptr;
    const float v = strtof(c, &end);
    if (end == c || *end != '\0' || !isfinite(v)) return false;
    out = v;
    return true;
}

static String settingsJson(const ControllerSettings &s) {
    String body = "{\"ok\":true,\"coolingTempC\":";
    body += String(s.coolingTempC, 1);
    body += ",\"darkLightPct\":" + String(s.darkLightPct, 1);
    body += ",\"revision\":" + String(s.revision);
    body += ",\"warningTempC\":" + String(TEMP_WARNING, 1) + "}";
    return body;
}

static void handleGetConfig() {
    ControllerSettings s = {};
    if (!readControllerSettings(s)) { sendError(503, "configuration busy"); return; }
    preventCache();
    server.send(200, "application/json", settingsJson(s));
}

static void handleSetConfig() {
    float cooling = 0, light = 0;
    if (!parseFloatField("coolingTempC", cooling) ||
        !parseFloatField("darkLightPct", light) ||
        cooling < TEMP_COOLING_MIN || cooling > TEMP_COOLING_MAX ||
        light < LIGHT_DARK_MIN || light > LIGHT_DARK_MAX) {
        sendError(400, "invalid threshold range");
        return;
    }
    // Tu choi moi truong la de phong truong hop web gui them warningTempC.
    if (server.hasArg("warningTempC")) {
        sendError(400, "WARNING 45C is fixed");
        return;
    }
    ControllerSettings updated = {};
    if (!updateControllerSettings(cooling, light, updated)) {
        sendError(503, "configuration busy");
        return;
    }
    preventCache();
    server.send(200, "application/json", settingsJson(updated));
    Serial.printf("[CONFIG] rev=%lu  COOLING >=%.1f C  LIGHTING <%.1f %%  WARNING >=45 C (fixed)\r\n",
       (unsigned long)updated.revision, updated.coolingTempC, updated.darkLightPct);
}

static void handleState() {
    SensorData sensor = {};
    DecisionData decision = {};
    ControllerSettings settings = {};
    if (!readControllerSettings(settings)) { sendError(503, "configuration busy"); return; }
    const bool sensorReady = xQueuePeek(sensorQueue, &sensor, 0) == pdTRUE;
    const bool decisionReady = xQueuePeek(decisionQueue, &decision, 0) == pdTRUE;
    const int fanPercent = (int)((fanPwm / 255.0f) * 100.0f);
    float deadlineRate = 0.0f;
    if (controlCycles > 0) {
        deadlineRate = 100.0f * (controlCycles - controlDeadlineMiss) / controlCycles;
    }

    String json = "{";
    json += "\"sensorReady\":" + String(sensorReady ? "true" : "false") + ",";
    json += "\"decisionReady\":" + String(decisionReady ? "true" : "false") + ",";
    json += "\"source\":\"REAL\",";
    json += "\"temperature\":" + String(sensor.temperature, 1) + ",";
    json += "\"humidity\":" + String(sensor.humidity, 1) + ",";
    json += "\"light\":" + String(sensor.light, 1) + ",";
    json += "\"lightLux\":" + String(sensor.ldrLux, 1) + ",";
    json += "\"ldrAdc\":" + String(sensor.ldrAdcRaw) + ",";
    json += "\"ldrVoltage\":" + String(sensor.ldrVoltageV, 3) + ",";
    json += "\"ldrResistance\":" + String(sensor.ldrResistanceKOhm, 2) + ",";
    json += "\"occupancy\":" + String(sensor.occupancy ? "true" : "false") + ",";
    json += "\"mode\":\"" + String(decisionReady ? modeName(decision.mode) : "PENDING") + "\",";
    json += "\"fan\":" + String(fanPercent) + ",";
    json += "\"lightOn\":" + String(lightOn ? "true" : "false") + ",";
    json += "\"alarmOn\":" + String(alarmOn ? "true" : "false") + ",";
    json += "\"pirLatency\":" + String(pirLatencyMs) + ",";
    json += "\"responseTime\":" + String(responseTimeMs) + ",";
    json += "\"controlExec\":" + String(controlExecUs / 1000.0f, 3) + ",";
    json += "\"deadlineRate\":" + String(deadlineRate, 1) + ",";
    json += "\"controlSamples\":" + String(controlCycles) + ",";
    json += "\"coolingTempC\":" + String(settings.coolingTempC, 1) + ",";
    json += "\"darkLightPct\":" + String(settings.darkLightPct, 1) + ",";
    json += "\"warningTempC\":" + String(TEMP_WARNING, 1) + ",";
    json += "\"configRevision\":" + String(settings.revision) + ",";
    json += "\"appliedRevision\":" + String(decisionReady ? decision.settings.revision : 0);
    json += "}";
    preventCache();
    server.send(200, "application/json", json);
}

void WebTask(void *parameter) {
    WiFi.begin("Wokwi-GUEST", "", 6);
    while (WiFi.status() != WL_CONNECTED) {
        vTaskDelay(pdMS_TO_TICKS(250));
    }
    Serial.println("[WEB] WiFi connected; HTTP port 80 (monitor REAL + config)");
    server.on("/", HTTP_GET, handleRoot);
    server.on("/api/state", HTTP_GET, handleState);
    server.on("/api/config", HTTP_GET, handleGetConfig);
    server.on("/api/config", HTTP_POST, handleSetConfig);
    server.begin();
    Serial.println("[WEB] Dashboard: http://localhost:8180");
    while (true) {
        server.handleClient();
        vTaskDelay(pdMS_TO_TICKS(2));
    }
}
