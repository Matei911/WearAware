#pragma once

#include "SensorReadings.h"

namespace BleEnvironment
{
bool start(const SensorReadings& initialReadings);
void publish(const SensorReadings& readings);
void stop();
bool isRunning();
}  // namespace BleEnvironment
