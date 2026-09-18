#include <Arduino.h>

#include "../core/config.h"
#include "../core/types.h"
#include "../core/app_state.h"
#include "control.h"

void initControl() {
    pinMode(LIGHT_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);

    ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
    ledcAttachPin(FAN_PIN, PWM_CHANNEL);

    digitalWrite(LIGHT_PIN, LOW);
    ledcWrite(PWM_CHANNEL, 0);
    noTone(BUZZER_PIN);
}

void ControlTask(void *parameter) {
    SensorData sensor;
    DecisionData decision;

    while (true) {
        TickType_t startTick = xTaskGetTickCount();

        if (xQueuePeek(sensorQueue, &sensor, 0) == pdTRUE &&
            xQueuePeek(decisionQueue, &decision, 0) == pdTRUE) {

            int nextFan = 0;
            bool nextLight = false;
            bool nextAlarm = false;

            switch (decision.mode) {

                case COMFORT:
                case ENERGY_SAVING:
                    break;

                case LIGHTING:
                    nextLight = true;
                    break;

                case COOLING:
                    nextFan = (sensor.temperature >= TEMP_FAN_MAX) ? 255 : 180;

                    // Nóng + tối + có người -> vừa làm mát vừa bật đèn
                    if (sensor.occupancy && sensor.light < LIGHT_DARK)
                        nextLight = true;
                    break;

                case WARNING:
                    nextFan = 255;
                    nextAlarm = true;
                    break;
            }

            // Ghi ra phần cứng
            digitalWrite(LIGHT_PIN, nextLight ? HIGH : LOW);
            ledcWrite(PWM_CHANNEL, nextFan);

            if (nextAlarm)
                tone(BUZZER_PIN, 1000);
            else
                noTone(BUZZER_PIN);

            // Lưu trạng thái để OLED/Web hiển thị
            fanPwm = nextFan;
            lightOn = nextLight;
            alarmOn = nextAlarm;

            // Sensor sample -> actuator update
            TickType_t now = xTaskGetTickCount();
            responseTimeMs = (now - sensor.sampleTick) * portTICK_PERIOD_MS;
        }

        // Đo thời gian thực thi ControlTask
        TickType_t endTick = xTaskGetTickCount();
        controlExecMs = (endTick - startTick) * portTICK_PERIOD_MS;

        controlCycles++;
        if (controlExecMs > CONTROL_DEADLINE_MS)
            controlDeadlineMiss++;

        vTaskDelay(pdMS_TO_TICKS(CONTROL_PERIOD_MS));
    }
}
