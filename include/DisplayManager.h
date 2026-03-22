#pragma once

#include "SamplingMenu.h"
#include "SamplingSettings.h"
#include "SensorReadings.h"

namespace DisplayManager
{
void init();
void forceNextFullRefresh();
void renderHomeScreen(const SensorReadings& readings);
void renderWaitingForDataScreen(const SensorReadings& readings);
void renderModeMenu(SamplingMenu::ModeRow selectedRow,
                    bool usePartialRefresh,
                    const SensorReadings& readings);
void renderDeepSleepMenu(SamplingMenu::DeepSleepRow selectedRow,
                         bool usePartialRefresh,
                         const SensorReadings& readings);
void renderAppConnectMenu(SamplingMenu::AppDurationRow selectedRow,
                          bool usePartialRefresh,
                          const SensorReadings& readings);
void renderPhonePromptScreen(const SensorReadings& readings);
}  // namespace DisplayManager
