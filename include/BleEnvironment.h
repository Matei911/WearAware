#pragma once

#include "SensorReadings.h"

namespace BleEnvironment
{
bool start();
void publish(const SensorReadings& readings);
void stop();
bool isRunning();
bool isConnected();
}  // namespace BleEnvironment
