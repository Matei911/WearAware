#pragma once

#include <Arduino.h>

enum class DeviceInterval : uint8_t
{
   FiveMinutes = 0,
   ThreeMinutes = 1,
   OneMinute = 2
};

enum class AppInterval : uint8_t
{
   SixtySeconds = 0,
   ThirtySeconds = 1,
   FifteenSeconds = 2
};

namespace ModeSettings
{
void init();
DeviceInterval getDeviceInterval();
void setDeviceInterval(DeviceInterval interval);
uint64_t selectedDeviceSleepDurationUs();
AppInterval getAppInterval();
void setAppInterval(AppInterval interval);
uint32_t selectedAppUpdateIntervalMs();
}  // namespace ModeSettings
