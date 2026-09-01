# Module Map

## 1. Initialization

```text
main.cpp
  │
  ├── initSensors()       -> sensors/
  ├── initControl()       -> control/
  ├── initDisplay()       -> display/
  ├── initRTOSObjects()   -> core/
  ├── initPIRInterrupt()  -> pir/
  │
  └── xTaskCreate(...)
       ├── PIRTask
       ├── ControlTask
       ├── SensorTask
       ├── DecisionTask
       ├── DisplayTask
       └── WebTask
```

## 2. Main control path

```text
sensors/
SensorTask
    ↓ SensorData
sensorQueue
    ↓
decision/
DecisionTask
    ↓ SystemMode
decisionQueue
    ↓
control/
ControlTask
    ↓
Relay / Fan PWM / Buzzer
```

## 3. PIR interrupt path

```text
PIR GPIO16
    ↓
Hardware Interrupt
    ↓
pirISR()
    ↓
xSemaphoreGiveFromISR()
    ↓
pirSemaphore
    ↓
PIRTask
```

## 4. Shared FreeRTOS objects

Located in `src/core/app_state.*`:

- `sensorQueue`
- `decisionQueue`
- `pirSemaphore`
- `webMutex`
- actuator state
- timing metrics

## 5. UI paths

```text
DisplayTask -> OLED SSD1306
WebTask     -> Wi-Fi / HTTP / browser dashboard
```
