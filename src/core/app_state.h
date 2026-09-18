#pragma once

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "types.h"

// FreeRTOS objects
extern QueueHandle_t sensorQueue;
extern QueueHandle_t decisionQueue;
extern SemaphoreHandle_t pirSemaphore;
extern SemaphoreHandle_t webMutex;

// Shared system state
extern volatile bool currentOccupancy;
extern volatile InputSource inputSource;
extern volatile TickType_t pirIsrTick;

// WEB sensor values protected by webMutex
extern SensorData webData;

// Current actuator states
extern volatile int fanPwm;
extern volatile bool lightOn;
extern volatile bool alarmOn;

// Real-time metrics
extern volatile uint32_t pirLatencyMs;
extern volatile uint32_t responseTimeMs;
extern volatile uint32_t controlExecMs;
extern volatile uint32_t controlCycles;
extern volatile uint32_t controlDeadlineMiss;

// Create Queue / Semaphore / Mutex and seed initial values
bool initRTOSObjects();
