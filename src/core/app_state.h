#pragma once
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "types.h"

extern QueueHandle_t sensorQueue;
extern QueueHandle_t decisionQueue;
extern SemaphoreHandle_t pirSemaphore;
extern SemaphoreHandle_t configMutex;

extern volatile bool currentOccupancy;
extern volatile TickType_t pirIsrTick;

// Config chia se giua WebTask, DecisionTask: doc/ghi qua configMutex.
extern ControllerSettings controllerSettings;
bool readControllerSettings(ControllerSettings &out);
bool updateControllerSettings(float coolingTempC, float darkLightPct,
                              ControllerSettings &out);

extern volatile int fanPwm;
extern volatile bool lightOn;
extern volatile bool alarmOn;

extern volatile uint32_t pirLatencyMs;
extern volatile uint32_t responseTimeMs;
extern volatile uint32_t controlExecMs;
extern volatile uint32_t controlExecUs;
extern volatile uint32_t controlCycles;
extern volatile uint32_t controlDeadlineMiss;

bool initRTOSObjects();
