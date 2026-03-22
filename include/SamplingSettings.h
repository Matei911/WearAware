#pragma once

#include <Arduino.h>

enum class SamplingInterval : uint8_t
{
   FiveMinutes = 0,
   ThreeMinutes = 1,
   OneMinute = 2
};

namespace SamplingSettings
{
void init();
SamplingInterval getSelectedInterval();
void setSelectedInterval(SamplingInterval interval);
SamplingInterval nextInterval(SamplingInterval interval);
SamplingInterval previousInterval(SamplingInterval interval);
uint64_t selectedSleepDurationUs();
}  // namespace SamplingSettings
