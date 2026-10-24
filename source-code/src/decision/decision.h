#pragma once
#include "../core/types.h"

SystemMode evaluateMode(const SensorData &data, const ControllerSettings &settings);
const char* modeName(SystemMode mode);
void DecisionTask(void *parameter);
