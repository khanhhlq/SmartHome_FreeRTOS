#pragma once

// =========================
// GPIO mapping
// =========================
#define DHT_PIN      15
#define LDR_PIN      4
#define PIR_PIN      16

#define LIGHT_PIN    6
#define FAN_PIN      17
#define BUZZER_PIN   7

#define OLED_SDA     8
#define OLED_SCL     9

// =========================
// Rule-based thresholds
// Priority:
// WARNING > ENERGY_SAVING > COOLING > LIGHTING > COMFORT
// =========================
constexpr float TEMP_WARNING = 45.0f;
constexpr float TEMP_COOLING = 30.0f;
constexpr float TEMP_FAN_MAX = 35.0f;
constexpr float LIGHT_DARK   = 30.0f;

// =========================
// Timing
// =========================
constexpr uint32_t SENSOR_REAL_PERIOD_MS = 2000;
constexpr uint32_t SENSOR_WEB_PERIOD_MS  = 500;
constexpr uint32_t DECISION_PERIOD_MS    = 100;
constexpr uint32_t CONTROL_PERIOD_MS     = 200;
constexpr uint32_t DISPLAY_PERIOD_MS     = 500;

constexpr uint32_t CONTROL_DEADLINE_MS   = 200;

// =========================
// PWM
// =========================
constexpr int PWM_CHANNEL    = 0;
constexpr int PWM_FREQ       = 5000;
constexpr int PWM_RESOLUTION = 8;
