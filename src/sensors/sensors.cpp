#include <Arduino.h>
#include <DHTesp.h>

#include "../core/config.h"
#include "../core/types.h"
#include "../core/app_state.h"
#include "sensors.h"

static DHTesp dht;

void initSensors() {
    dht.setup(DHT_PIN, DHTesp::DHT22);
    pinMode(LDR_PIN, INPUT);
    pinMode(PIR_PIN, INPUT);
}

void SensorTask(void *parameter) {
    SensorData data = {25.0f, 55.0f, 65.0f, false, 0};

    while (true) {

        // ===== WEB MODE =====
        if (inputSource == WEB_SENSOR) {
            if (xSemaphoreTake(webMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
                data = webData;
                xSemaphoreGive(webMutex);
            }

            data.sampleTick = xTaskGetTickCount();
            xQueueOverwrite(sensorQueue, &data);

            vTaskDelay(pdMS_TO_TICKS(SENSOR_WEB_PERIOD_MS));
            continue;
        }

        // ===== REAL SENSOR MODE =====
        TempAndHumidity th = dht.getTempAndHumidity();

        // Chỉ ghi đè khi DHT22 trả về dữ liệu hợp lệ
        if (!isnan(th.temperature)) data.temperature = th.temperature;
        if (!isnan(th.humidity))    data.humidity = th.humidity;

        int raw = analogRead(LDR_PIN);
        data.light = (raw / 4095.0f) * 100.0f;

        data.occupancy = currentOccupancy;
        data.sampleTick = xTaskGetTickCount();

        // Queue length = 1 -> luôn giữ mẫu mới nhất
        xQueueOverwrite(sensorQueue, &data);

        vTaskDelay(pdMS_TO_TICKS(SENSOR_REAL_PERIOD_MS));
    }
}
