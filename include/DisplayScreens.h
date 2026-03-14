#pragma once

#include "DisplayTypes.h"
#include "SamplingSettings.h"
#include "SensorReadings.h"

namespace ScreenHeader
{
void draw(WearAwareDisplay& display, float batteryPercent);
}  // namespace ScreenHeader

namespace HomeScreen
{
void draw(WearAwareDisplay& display, const SensorReadings& readings);
}  // namespace HomeScreen

namespace SamplingMenuScreen
{
void draw(WearAwareDisplay& display,
          SamplingInterval selectedInterval,
          const SensorReadings& readings);
}  // namespace SamplingMenuScreen
