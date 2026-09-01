#pragma once

#include "../core/types.h"

SystemMode evaluateMode(const SensorData &data);
const char* modeName(SystemMode mode);
const char* sourceName();

void DecisionTask(void *parameter);
