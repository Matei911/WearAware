#pragma once

#include <Arduino.h>

#include "BleSettings.h"
#include "SensorReadings.h"
#include "SamplingSettings.h"

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

struct MenuSelection
{
   ModeRow mode;
   SamplingInterval deepSleepInterval;
   BleSettings::UpdateInterval bleUpdateInterval;

   MenuSelection()
       : mode(ModeRow::DeepSleepSample),
         deepSleepInterval(SamplingInterval::FiveMinutes),
         bleUpdateInterval(BleSettings::UpdateInterval::SixtySeconds)
   {
   }

   MenuSelection(ModeRow selectedMode,
                 SamplingInterval selectedDeepSleepInterval,
                 BleSettings::UpdateInterval selectedBleUpdateInterval)
       : mode(selectedMode),
         deepSleepInterval(selectedDeepSleepInterval),
         bleUpdateInterval(selectedBleUpdateInterval)
   {
   }
};

MenuSelection run(const SensorReadings& readings);
}  // namespace SamplingMenu
