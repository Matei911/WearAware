#include "BleSettings.h"

#include <Arduino.h>

namespace
{
constexpr uint8_t BLE_DISABLED = 0;
constexpr uint8_t BLE_ENABLED = 1;

RTC_DATA_ATTR uint8_t bleEnabledState = BLE_DISABLED;

uint8_t sanitizeState(uint8_t state)
{
   return state == BLE_ENABLED ? BLE_ENABLED : BLE_DISABLED;
}
}  // namespace

namespace BleSettings
{
void init()
{
   bleEnabledState = sanitizeState(bleEnabledState);
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
}  // namespace BleSettings
