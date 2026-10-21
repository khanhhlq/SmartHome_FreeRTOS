#include <Arduino.h>

#include "../core/config.h"
#include "../core/types.h"
#include "../core/app_state.h"
#include "control.h"

void initControl() {
    pinMode(LIGHT_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);

    // PlatformIO espressif32@6.12.0 -> Arduino-ESP32 2.0.17 APIs.
    ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
    ledcAttachPin(FAN_PIN, PWM_CHANNEL);

    digitalWrite(LIGHT_PIN, RELAY_ACTIVE_HIGH ? LOW : HIGH);
    ledcWrite(PWM_CHANNEL, 0);
    noTone(BUZZER_PIN);
}

void ControlTask(void *parameter) {
    DecisionData decision;
    TickType_t lastAppliedTick = portMAX_DELAY;
    bool buzzerWasOn = false;

    while (true) {
        // Process new decision immediately, instead of polling every 200 ms.
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        const uint32_t startUs = micros();

        if (xQueuePeek(decisionQueue, &decision, 0) == pdTRUE) {
            // Use the sensor snapshot evaluated by DecisionTask, not another
            // sample from sensorQueue (which might belong to a newer update).
            const SensorData &sensor = decision.sensor;
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
                    nextFan = sensor.temperature >= TEMP_FAN_MAX ? 255 : 180;
                    if (sensor.occupancy && sensor.light < decision.settings.darkLightPct) {
                        nextLight = true;
                    }
                    break;
                case WARNING:
                    nextFan = 255;
                    nextAlarm = true;
                    break;
            }

            digitalWrite(LIGHT_PIN, (nextLight == RELAY_ACTIVE_HIGH) ? HIGH : LOW);
            ledcWrite(PWM_CHANNEL, nextFan);
            // Avoid re-configuring the tone peripheral on every 200-ms cycle.
            if (nextAlarm != buzzerWasOn) {
                if (nextAlarm) tone(BUZZER_PIN, 1000);
                else noTone(BUZZER_PIN);
                buzzerWasOn = nextAlarm;
            }

            fanPwm = nextFan;
            lightOn = nextLight;
            alarmOn = nextAlarm;

            // Chi dem mau cam bien moi, khong dem lai cung mot mau vao deadline.
            if (sensor.sampleTick != lastAppliedTick) {
                lastAppliedTick = sensor.sampleTick;
                const TickType_t now = xTaskGetTickCount();
                responseTimeMs = (now - sensor.sampleTick) * portTICK_PERIOD_MS;
                if (responseTimeMs > maxResponseTimeMs) maxResponseTimeMs = responseTimeMs;
                controlCycles++;
                if (responseTimeMs > CONTROL_DEADLINE_MS) {
                    controlDeadlineMiss++;
                }
            }
        }

        controlExecUs = micros() - startUs;
        controlExecMs = controlExecUs / 1000; // giữ tương thích với mã cũ
    }
}
