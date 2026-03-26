#include "BleSettings.h"

#include <Arduino.h>

namespace
{
constexpr uint8_t BLE_DISABLED = 0;
constexpr uint8_t BLE_ENABLED = 1;
constexpr uint32_t UPDATE_INTERVAL_MS[] = {
    60000UL,
    30000UL,
    15000UL,
};
constexpr uint8_t UPDATE_INTERVAL_COUNT =
    sizeof(UPDATE_INTERVAL_MS) / sizeof(UPDATE_INTERVAL_MS[0]);

RTC_DATA_ATTR uint8_t bleEnabledState = BLE_DISABLED;
RTC_DATA_ATTR uint8_t updateIntervalIndex =
    static_cast<uint8_t>(BleSettings::UpdateInterval::SixtySeconds);

uint8_t sanitizeState(uint8_t state)
{
   return state == BLE_ENABLED ? BLE_ENABLED : BLE_DISABLED;
}

uint8_t sanitizeIntervalIndex(uint8_t index)
{
   return index < UPDATE_INTERVAL_COUNT ? index : 0;
}
}  // namespace

namespace BleSettings
{
void init()
{
   bleEnabledState = sanitizeState(bleEnabledState);
   updateIntervalIndex = sanitizeIntervalIndex(updateIntervalIndex);
}

bool isEnabled()
{
   return bleEnabledState == BLE_ENABLED;
}

void setEnabled(bool enabled)
{
   bleEnabledState = enabled ? BLE_ENABLED : BLE_DISABLED;
}

void toggle()
{
   setEnabled(!isEnabled());
}

UpdateInterval getUpdateInterval()
{
   return static_cast<UpdateInterval>(
       sanitizeIntervalIndex(updateIntervalIndex));
}

void setUpdateInterval(UpdateInterval interval)
{
   updateIntervalIndex =
       sanitizeIntervalIndex(static_cast<uint8_t>(interval));
}

uint32_t selectedUpdateIntervalMs()
{
   return UPDATE_INTERVAL_MS[updateIntervalIndex];
}
}  // namespace BleSettings
