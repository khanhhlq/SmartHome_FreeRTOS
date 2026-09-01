# Smart Home Environmental Control System
## ESP32-S3 + FreeRTOS + FSM / Rule-Based Control

Final project version uses deterministic **FSM / Rule-Based Control** for all decisions.

The project is organized by functional modules so each embedded-system concept can be studied independently.

## Project structure

```text
SmartHome_FreeRTOS_Final/
├── src/
│   ├── main.cpp
│   ├── core/
│   │   ├── config.h
│   │   ├── types.h
│   │   ├── app_state.h
│   │   └── app_state.cpp
│   ├── sensors/
│   │   ├── sensors.h
│   │   └── sensors.cpp
│   ├── decision/
│   │   ├── decision.h
│   │   └── decision.cpp
│   ├── control/
│   │   ├── control.h
│   │   └── control.cpp
│   ├── pir/
│   │   ├── pir.h
│   │   └── pir.cpp
│   ├── display/
│   │   ├── display.h
│   │   └── display.cpp
│   └── web/
│       ├── web_server.h
│       ├── web_server.cpp
│       └── web_page.h
├── docs/
│   └── MODULE_MAP.md
├── diagram.json
├── platformio.ini
├── wokwi.toml
└── .gitignore
```

## Main data flow

```text
DHT22 / LDR / PIR or Web Input
              ↓
          SensorTask
              ↓
          sensorQueue
              ↓
         DecisionTask
          FSM / Rules
              ↓
         decisionQueue
              ↓
          ControlTask
        ┌─────┼──────┐
        ↓     ↓      ↓
      Relay  Fan   Buzzer
```

PIR event path:

```text
PIR
 ↓
Hardware Interrupt
 ↓
ISR
 ↓
Binary Semaphore
 ↓
PIRTask
```

## FreeRTOS task priorities

| Task | Priority | Role |
|---|---:|---|
| PIRTask | 4 | Handle PIR interrupt events |
| ControlTask | 3 | Update actuators |
| SensorTask | 2 | Read sensors / WEB values |
| DecisionTask | 2 | Run FSM / rule-based decision |
| DisplayTask | 1 | Update OLED |
| WebTask | 1 | HTTP dashboard |

## FSM / rule priority

1. `temperature >= 45°C` → `WARNING`
2. No occupancy → `ENERGY_SAVING`
3. `temperature >= 30°C` → `COOLING`
4. `light < 30%` → `LIGHTING`
5. Otherwise → `COMFORT`

## Real-time evaluation

The firmware exposes:

- PIR ISR → PIRTask latency
- Sensor sample → actuator response time
- ControlTask execution time
- Control deadline success rate

These are **Wokwi simulation measurements** and should not be treated as guaranteed real-hardware timing.

## Web Dashboard

The ESP32-S3 serves an HTTP dashboard on port 80. `wokwi.toml` forwards it to:

```text
http://localhost:8180
```

The dashboard supports:

- REAL mode: DHT22 + LDR + PIR
- WEB mode: browser-provided temperature, humidity, light and occupancy
- System mode display
- Light / fan / buzzer state
- Real-time timing metrics

## Suggested study order

1. `src/main.cpp`
2. `src/core/config.h` and `src/core/types.h`
3. `src/sensors/`
4. `src/decision/`
5. `src/control/`
6. `src/pir/`
7. `src/core/app_state.*`
8. `src/display/`
9. `src/web/`
