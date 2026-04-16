#pragma once

#include <Arduino.h>

#include "ModeSettings.h"
#include "SensorReadings.h"

namespace SamplingMenu
{
enum class ModeRow : uint8_t
{
   DeviceMode = 0,
   AppMode = 1
};

enum class DeviceIntervalRow : uint8_t
{
   FiveMinutes = 0,
   ThreeMinutes = 1,
   OneMinute = 2
};

enum class AppIntervalRow : uint8_t
{
   SixtySeconds = 0,
   ThirtySeconds = 1,
   FifteenSeconds = 2
};

struct MenuSelection
{
   ModeRow mode;
   DeviceInterval deviceInterval;
   AppInterval appInterval;

   MenuSelection()
       : mode(ModeRow::DeviceMode),
         deviceInterval(DeviceInterval::FiveMinutes),
         appInterval(AppInterval::SixtySeconds)
   {
   }

   MenuSelection(ModeRow selectedMode,
                 DeviceInterval selectedDeviceInterval,
                 AppInterval selectedAppInterval)
       : mode(selectedMode),
         deviceInterval(selectedDeviceInterval),
         appInterval(selectedAppInterval)
   {
   }
};

MenuSelection run(const SensorReadings& readings);
}  // namespace SamplingMenu
