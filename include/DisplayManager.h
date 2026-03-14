#pragma once

#include "SensorReadings.h"
#include "SamplingSettings.h"

namespace DisplayManager
{
void init();
void forceNextFullRefresh();
void renderHomeScreen(const SensorReadings& readings);
void renderWaitingForDataScreen(const SensorReadings& readings);
void renderSamplingMenu(SamplingInterval selectedInterval,
                        bool usePartialRefresh, const SensorReadings& readings);
}  // namespace DisplayManager
