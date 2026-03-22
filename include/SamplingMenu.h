#pragma once

#include <Arduino.h>

#include "SensorReadings.h"

namespace SamplingMenu
{
enum class ModeRow : uint8_t
{
   ConnectToApp = 0,
   DeepSleepSample = 1
};

enum class DeepSleepRow : uint8_t
{
   FiveMinutes = 0,
   ThreeMinutes = 1,
   OneMinute = 2
};

enum class AppDurationRow : uint8_t
{
   SixtySeconds = 0,
   ThirtySeconds = 1,
   FifteenSeconds = 2
};

void run(const SensorReadings& readings);
}  // namespace SamplingMenu
