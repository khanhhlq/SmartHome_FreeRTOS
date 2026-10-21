#include <Arduino.h>
#include <DHTesp.h>
#include <math.h>

#include "../core/config.h"
#include "../core/types.h"
#include "../core/app_state.h"
#include "sensors.h"
#include "light_math.h"

static DHTesp dht;

void initSensors() {
    dht.setup(DHT_PIN, DHTesp::DHT22);
    pinMode(LDR_PIN, INPUT);
    pinMode(PIR_PIN, INPUT);
    analogReadResolution(12); // 12 bit -> 0..4095 (matches report section 2.2.4).
    Serial.println("[INFO] DHT22: DHTesp decodes 40-bit frames to C and %RH; PIR: digital GPIO16.");
    Serial.println("[INFO] LDR: analogRead(GPIO4) -> model(V,R,lux) -> Light(%) for FSM.");
}

void SensorTask(void *parameter) {
    SensorData data = {};
    data.temperature = NAN;  // Chua co mau hop le thi khong tu tao gia tri
    data.humidity = NAN;
    TickType_t lastLog = 0;
    bool firstLog = true;

    while (true) {
        TempAndHumidity th = dht.getTempAndHumidity();
        if (isfinite(th.temperature)) data.temperature = th.temperature;
        if (isfinite(th.humidity)) data.humidity = th.humidity;

        const int raw = analogRead(LDR_PIN);
        const LdrDiagnostics ldr = ldrFromAdc(raw);
        data.ldrAdcRaw = ldr.adcRaw;
        data.ldrVoltageV = ldr.voltageV;   // tinh theo mo hinh, khong phai analogReadMilliVolts
        data.ldrResistanceKOhm = ldr.resistanceKOhm;
        data.ldrLux = ldr.lux;
        data.light = ldr.brightnessPct;
        data.occupancy = currentOccupancy;
        data.sampleTick = xTaskGetTickCount();

        // DHT22 chua san sang -> khong dua NAN vao FSM hay JSON.
        if (!isfinite(data.temperature) || !isfinite(data.humidity)) {
            Serial.println("[SENSOR] DHT22 waiting for first valid measurement");
            vTaskDelay(pdMS_TO_TICKS(SENSOR_REAL_PERIOD_MS));
            continue;
        }
        xQueueOverwrite(sensorQueue, &data);
        // Wake DecisionTask for each new REAL sensor sample.
        if (decisionTaskHandle) xTaskNotifyGive(decisionTaskHandle);

        const TickType_t now = xTaskGetTickCount();
        if (firstLog || ((now-lastLog) * portTICK_PERIOD_MS >= REAL_LOG_PERIOD_MS)) {
            firstLog = false;
            lastLog = now;
            // CRLF la bat buoc voi terminal Wokwi: LF don le co the lam chu
            // "chay cheo" vi con tro xuong dong nhung khong ve cot dau.
            // 1 printf / 5s; DHT22 + PIR la du lieu cam bien;
            // ADC la gia tri doc; dien ap, R, lux la uoc tinh MO HINH,
            // LIGHT(%) la chi so quy uoc de FSM so sanh nguong.
            Serial.printf("[REAL] DHT22  T=%5.1f C  RH=%5.1f %%RH  PIR(GPIO16)=%d\r\n"
                          "       LDR    ADC=%4d/4095  AO(model)=%5.3f V  R(model)=%6.2f kOhm\r\n"
                          "       LDR    E(model)=%7.1f lux  Light(FSM)=%5.1f %%\r\n",
                          data.temperature, data.humidity, data.occupancy ? 1 : 0,
                          data.ldrAdcRaw, data.ldrVoltageV,
                          data.ldrResistanceKOhm, data.ldrLux, data.light);
        }
        vTaskDelay(pdMS_TO_TICKS(SENSOR_REAL_PERIOD_MS));
    }
}
