#pragma once

#include <Arduino.h>

namespace BleSettings
{
enum class UpdateInterval : uint8_t
{
   SixtySeconds = 0,
   ThirtySeconds = 1,
   FifteenSeconds = 2
};

void init();
bool isEnabled();
void setEnabled(bool enabled);
void toggle();
UpdateInterval getUpdateInterval();
void setUpdateInterval(UpdateInterval interval);
uint32_t selectedUpdateIntervalMs();
}  // namespace BleSettings
