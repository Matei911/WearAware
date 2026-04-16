#pragma once

#include "SamplingMenu.h"
#include "SensorReadings.h"

namespace DisplayManager
{
void init();
void wake();
void sleep();
void forceNextFullRefresh();
void renderHomeScreen(const SensorReadings& readings);
void renderModeMenu(SamplingMenu::ModeRow selectedRow,
                    bool usePartialRefresh,
                    const SensorReadings& readings);
void renderDeviceIntervalMenu(
    SamplingMenu::DeviceIntervalRow selectedRow,
    bool usePartialRefresh,
    const SensorReadings& readings);
void renderAppIntervalMenu(SamplingMenu::AppIntervalRow selectedRow,
                           bool usePartialRefresh,
                           const SensorReadings& readings);
void renderPhonePromptScreen(const SensorReadings& readings);
}  // namespace DisplayManager
