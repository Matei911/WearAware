#pragma once

#include "DisplayTypes.h"
#include "SamplingMenu.h"
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
void drawModeMenu(WearAwareDisplay& display,
                  SamplingMenu::ModeRow selectedRow,
                  const SensorReadings& readings);
void drawDeviceIntervalMenu(
    WearAwareDisplay& display,
    SamplingMenu::DeviceIntervalRow selectedRow,
    const SensorReadings& readings);
void drawAppIntervalMenu(WearAwareDisplay& display,
                         SamplingMenu::AppIntervalRow selectedRow,
                         const SensorReadings& readings);
}  // namespace SamplingMenuScreen

namespace WaitingForDataScreen
{
void draw(WearAwareDisplay& display, const SensorReadings& readings);
}  // namespace WaitingForDataScreen

namespace PhonePromptScreen
{
void draw(WearAwareDisplay& display, const SensorReadings& readings);
}  // namespace PhonePromptScreen

namespace ConnectedPromptScreen
{
void draw(WearAwareDisplay& display, const SensorReadings& readings);
}  // namespace ConnectedPromptScreen
