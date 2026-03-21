#pragma once

#include <Arduino.h>

#include "SensorReadings.h"

namespace SamplingMenu
{
enum class MenuRow : uint8_t
{
   FiveMinutes = 0,
   OneMinute = 1,
   ThirtySeconds = 2,
   BleToggle = 3
};

void run(const SensorReadings& readings);
MenuRow selectedRow();
}  // namespace SamplingMenu
