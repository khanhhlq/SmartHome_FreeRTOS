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
constexpr float TEMP_COOLING_DEFAULT = 30.0f;
constexpr float TEMP_COOLING_MIN = 25.0f;
constexpr float TEMP_COOLING_MAX = 40.0f;
constexpr float TEMP_FAN_MAX = 35.0f;
constexpr float LIGHT_DARK_DEFAULT = 30.0f;
constexpr float LIGHT_DARK_MIN = 10.0f;
constexpr float LIGHT_DARK_MAX = 60.0f;

// Wokwi LDR AO falls as illumination rises. Controller semantics:
// 0% = dark, 100% = bright; light < 30% therefore means dark.
constexpr bool LDR_ADC_INVERTED_TO_BRIGHTNESS = true;

// The relay should turn on with HIGH as described in report(6).
// Confirm polarity in Wokwi, because simulation/module variants can invert it.
constexpr bool RELAY_ACTIVE_HIGH = true;

// =========================
// Timing
// =========================
constexpr uint32_t SENSOR_REAL_PERIOD_MS = 2000;
constexpr uint32_t REAL_LOG_PERIOD_MS = 5000;
constexpr uint32_t DECISION_PERIOD_MS    = 50;  // fallback for missed notifications
constexpr uint32_t CONTROL_PERIOD_MS     = 50;  // retained for compatibility; ControlTask waits for notifications
constexpr uint32_t DISPLAY_PERIOD_MS     = 500;

constexpr uint32_t CONTROL_DEADLINE_MS   = 200;

// =========================
// PWM
// =========================
constexpr int PWM_CHANNEL    = 0;
constexpr int PWM_FREQ       = 5000;
constexpr int PWM_RESOLUTION = 8;
