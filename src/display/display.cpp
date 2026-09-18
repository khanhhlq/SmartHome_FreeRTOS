#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "../core/config.h"
#include "../core/types.h"
#include "../core/app_state.h"
#include "../decision/decision.h"
#include "display.h"

static Adafruit_SSD1306 display(128, 64, &Wire, -1);

void initDisplay() {
    Wire.begin(OLED_SDA, OLED_SCL);

    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("[OLED] Init failed");
        return;
    }

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Smart Home");
    display.println("FreeRTOS + FSM");
    display.display();
}

void DisplayTask(void *parameter) {
    SensorData sensor;
    DecisionData decision;

    while (true) {
        if (xQueuePeek(sensorQueue, &sensor, 0) == pdTRUE &&
            xQueuePeek(decisionQueue, &decision, 0) == pdTRUE) {

            int fanPercent = (int)((fanPwm / 255.0f) * 100.0f);

            display.clearDisplay();
            display.setTextSize(1);
            display.setTextColor(SSD1306_WHITE);
            display.setCursor(0, 0);

            display.print("SRC: ");
            display.println(sourceName());

            display.print("T:");
            display.print(sensor.temperature, 1);
            display.print(" H:");
            display.println(sensor.humidity, 0);

            display.print("Light:");
            display.print(sensor.light, 0);
            display.print("% PIR:");
            display.println(sensor.occupancy ? "Y" : "N");

            display.print("Mode:");
            display.println(modeName(decision.mode));

            display.print("Fan:");
            display.print(fanPercent);
            display.print("% L:");
            display.println(lightOn ? "ON" : "OFF");

            display.display();
        }

        vTaskDelay(pdMS_TO_TICKS(DISPLAY_PERIOD_MS));
    }
}
