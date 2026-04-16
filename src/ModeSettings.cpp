#include "ModeSettings.h"

namespace
{
constexpr uint64_t DEVICE_SLEEP_DURATIONS_US[] = {
    300ULL * 1000000ULL,
    180ULL * 1000000ULL,
    60ULL * 1000000ULL,
};

constexpr uint32_t APP_UPDATE_INTERVALS_MS[] = {
    60000UL,
    30000UL,
    15000UL,
};

constexpr uint8_t DEVICE_INTERVAL_COUNT =
    sizeof(DEVICE_SLEEP_DURATIONS_US) /
    sizeof(DEVICE_SLEEP_DURATIONS_US[0]);
constexpr uint8_t APP_INTERVAL_COUNT =
    sizeof(APP_UPDATE_INTERVALS_MS) /
    sizeof(APP_UPDATE_INTERVALS_MS[0]);

RTC_DATA_ATTR uint8_t selectedDeviceIntervalIndex =
    static_cast<uint8_t>(DeviceInterval::FiveMinutes);
RTC_DATA_ATTR uint8_t selectedAppIntervalIndex =
    static_cast<uint8_t>(AppInterval::SixtySeconds);

uint8_t sanitizeDeviceIntervalIndex(uint8_t index)
{
   return index < DEVICE_INTERVAL_COUNT ? index : 0;
}

uint8_t sanitizeAppIntervalIndex(uint8_t index)
{
   return index < APP_INTERVAL_COUNT ? index : 0;
}
}  // namespace

namespace ModeSettings
{
void init()
{
   selectedDeviceIntervalIndex =
       sanitizeDeviceIntervalIndex(selectedDeviceIntervalIndex);
   selectedAppIntervalIndex =
       sanitizeAppIntervalIndex(selectedAppIntervalIndex);
}

DeviceInterval getDeviceInterval()
{
   return static_cast<DeviceInterval>(
       sanitizeDeviceIntervalIndex(selectedDeviceIntervalIndex));
}

void setDeviceInterval(DeviceInterval interval)
{
   selectedDeviceIntervalIndex =
       sanitizeDeviceIntervalIndex(static_cast<uint8_t>(interval));
}

uint64_t selectedDeviceSleepDurationUs()
{
   return DEVICE_SLEEP_DURATIONS_US[selectedDeviceIntervalIndex];
}

AppInterval getAppInterval()
{
   return static_cast<AppInterval>(
       sanitizeAppIntervalIndex(selectedAppIntervalIndex));
}

void setAppInterval(AppInterval interval)
{
   selectedAppIntervalIndex =
       sanitizeAppIntervalIndex(static_cast<uint8_t>(interval));
}

uint32_t selectedAppUpdateIntervalMs()
{
   return APP_UPDATE_INTERVALS_MS[selectedAppIntervalIndex];
}
}  // namespace ModeSettings
