#pragma once

#include "SensorReadings.h"

namespace Sensors
{
void init();
void wake();
void sleep();
float batteryPercentForMenu();
float readBatteryPercent();
SensorReadings readAll();
}  // namespace Sensors
