#pragma once

#include "SensorReadings.h"

namespace Sensors
{
void init();
float batteryPercentForMenu();
float readBatteryPercent();
SensorReadings readAll();
}  // namespace Sensors
