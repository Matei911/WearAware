#include "SamplingMenu.h"

#include <Arduino.h>

#include "AppConfig.h"
#include "DisplayManager.h"
#include "ModeSettings.h"
#include "PowerManager.h"

namespace
{
constexpr uint16_t BUTTON_DEBOUNCE_MS = 300;
constexpr uint8_t MODE_ROW_COUNT = 2;
constexpr uint8_t DEVICE_INTERVAL_ROW_COUNT = 3;
constexpr uint8_t APP_INTERVAL_ROW_COUNT = 3;

enum class MenuState : uint8_t
{
   ChooseMode = 0,
   DeviceInterval = 1,
   AppInterval = 2
};

MenuState currentState = MenuState::ChooseMode;
SamplingMenu::ModeRow currentModeRow =
    SamplingMenu::ModeRow::DeviceMode;
SamplingMenu::DeviceIntervalRow currentDeviceIntervalRow =
    SamplingMenu::DeviceIntervalRow::FiveMinutes;
SamplingMenu::AppIntervalRow currentAppIntervalRow =
    SamplingMenu::AppIntervalRow::SixtySeconds;

bool buttonPressed(int pin)
{
   return digitalRead(pin) == LOW;
}

void waitForButtonRelease(int pin)
{
   while (digitalRead(pin) == LOW)
   {
      delay(10);
   }

   delay(BUTTON_DEBOUNCE_MS);
}

template <typename RowType>
RowType nextRow(RowType row, uint8_t rowCount)
{
   const uint8_t rowIndex = static_cast<uint8_t>(row);
   return static_cast<RowType>((rowIndex + 1) % rowCount);
}

template <typename RowType>
RowType previousRow(RowType row, uint8_t rowCount)
{
   const uint8_t rowIndex = static_cast<uint8_t>(row);
   return static_cast<RowType>((rowIndex + rowCount - 1) % rowCount);
}

SamplingMenu::DeviceIntervalRow rowFromDeviceInterval(
    DeviceInterval interval)
{
   switch (interval)
   {
      case DeviceInterval::FiveMinutes:
         return SamplingMenu::DeviceIntervalRow::FiveMinutes;
      case DeviceInterval::ThreeMinutes:
         return SamplingMenu::DeviceIntervalRow::ThreeMinutes;
      case DeviceInterval::OneMinute:
      default:
         return SamplingMenu::DeviceIntervalRow::OneMinute;
   }
}

DeviceInterval deviceIntervalFromRow(
    SamplingMenu::DeviceIntervalRow row)
{
   switch (row)
   {
      case SamplingMenu::DeviceIntervalRow::FiveMinutes:
         return DeviceInterval::FiveMinutes;
      case SamplingMenu::DeviceIntervalRow::ThreeMinutes:
         return DeviceInterval::ThreeMinutes;
      case SamplingMenu::DeviceIntervalRow::OneMinute:
      default:
         return DeviceInterval::OneMinute;
   }
}

SamplingMenu::AppIntervalRow rowFromAppInterval(AppInterval interval)
{
   switch (interval)
   {
      case AppInterval::SixtySeconds:
         return SamplingMenu::AppIntervalRow::SixtySeconds;
      case AppInterval::ThirtySeconds:
         return SamplingMenu::AppIntervalRow::ThirtySeconds;
      case AppInterval::FifteenSeconds:
      default:
         return SamplingMenu::AppIntervalRow::FifteenSeconds;
   }
}

AppInterval appIntervalFromRow(SamplingMenu::AppIntervalRow row)
{
   switch (row)
   {
      case SamplingMenu::AppIntervalRow::SixtySeconds:
         return AppInterval::SixtySeconds;
      case SamplingMenu::AppIntervalRow::ThirtySeconds:
         return AppInterval::ThirtySeconds;
      case SamplingMenu::AppIntervalRow::FifteenSeconds:
      default:
         return AppInterval::FifteenSeconds;
   }
}

SamplingMenu::MenuSelection makeDeviceModeSelection(
    SamplingMenu::DeviceIntervalRow row)
{
   SamplingMenu::MenuSelection selection;
   selection.mode = SamplingMenu::ModeRow::DeviceMode;
   selection.deviceInterval = deviceIntervalFromRow(row);
   selection.appInterval = ModeSettings::getAppInterval();
   return selection;
}

SamplingMenu::MenuSelection makeAppModeSelection(
    SamplingMenu::AppIntervalRow row)
{
   SamplingMenu::MenuSelection selection;
   selection.mode = SamplingMenu::ModeRow::AppMode;
   selection.deviceInterval = ModeSettings::getDeviceInterval();
   selection.appInterval = appIntervalFromRow(row);
   return selection;
}

void renderCurrentScreen(bool usePartialRefresh,
                         const SensorReadings& readings)
{
   switch (currentState)
   {
      case MenuState::ChooseMode:
         DisplayManager::renderModeMenu(currentModeRow,
                                        usePartialRefresh,
                                        readings);
         return;
      case MenuState::DeviceInterval:
         DisplayManager::renderDeviceIntervalMenu(
             currentDeviceIntervalRow,
             usePartialRefresh,
             readings);
         return;
      case MenuState::AppInterval:
         DisplayManager::renderAppIntervalMenu(currentAppIntervalRow,
                                               usePartialRefresh,
                                               readings);
         return;
      default:
         DisplayManager::renderModeMenu(currentModeRow,
                                        usePartialRefresh,
                                        readings);
         return;
   }
}
}  // namespace

namespace SamplingMenu
{
MenuSelection run(const SensorReadings& readings)
{
   currentState = MenuState::ChooseMode;
   currentModeRow = ModeRow::DeviceMode;
   currentDeviceIntervalRow =
       rowFromDeviceInterval(ModeSettings::getDeviceInterval());
   currentAppIntervalRow =
       rowFromAppInterval(ModeSettings::getAppInterval());

   DisplayManager::wake();
   renderCurrentScreen(false, readings);
   PowerManager::waitForButtonsReleased();
   delay(BUTTON_DEBOUNCE_MS);

   while (true)
   {
      if (buttonPressed(AppConfig::BUTTON_PIN_2))
      {
         switch (currentState)
         {
            case MenuState::ChooseMode:
               currentModeRow =
                   previousRow(currentModeRow, MODE_ROW_COUNT);
               renderCurrentScreen(true, readings);
               break;
            case MenuState::DeviceInterval:
               currentDeviceIntervalRow =
                   previousRow(currentDeviceIntervalRow,
                               DEVICE_INTERVAL_ROW_COUNT);
               renderCurrentScreen(true, readings);
               break;
            case MenuState::AppInterval:
               currentAppIntervalRow =
                   previousRow(currentAppIntervalRow,
                               APP_INTERVAL_ROW_COUNT);
               renderCurrentScreen(true, readings);
               break;
            default:
               break;
         }

         waitForButtonRelease(AppConfig::BUTTON_PIN_2);
         continue;
      }

      if (buttonPressed(AppConfig::BUTTON_PIN_3))
      {
         switch (currentState)
         {
            case MenuState::ChooseMode:
               currentModeRow = nextRow(currentModeRow, MODE_ROW_COUNT);
               renderCurrentScreen(true, readings);
               break;
            case MenuState::DeviceInterval:
               currentDeviceIntervalRow =
                   nextRow(currentDeviceIntervalRow,
                           DEVICE_INTERVAL_ROW_COUNT);
               renderCurrentScreen(true, readings);
               break;
            case MenuState::AppInterval:
               currentAppIntervalRow =
                   nextRow(currentAppIntervalRow,
                           APP_INTERVAL_ROW_COUNT);
               renderCurrentScreen(true, readings);
               break;
            default:
               break;
         }

         waitForButtonRelease(AppConfig::BUTTON_PIN_3);
         continue;
      }

      if (buttonPressed(AppConfig::BUTTON_PIN_1))
      {
         waitForButtonRelease(AppConfig::BUTTON_PIN_1);

         switch (currentState)
         {
            case MenuState::ChooseMode:
               currentState = currentModeRow == ModeRow::AppMode
                                  ? MenuState::AppInterval
                                  : MenuState::DeviceInterval;
               DisplayManager::forceNextFullRefresh();
               renderCurrentScreen(false, readings);
               break;
            case MenuState::DeviceInterval:
               DisplayManager::forceNextFullRefresh();
               return makeDeviceModeSelection(currentDeviceIntervalRow);
            case MenuState::AppInterval:
               DisplayManager::forceNextFullRefresh();
               return makeAppModeSelection(currentAppIntervalRow);
            default:
               break;
         }

         continue;
      }

      delay(20);
   }

   MenuSelection selection;
   return selection;
}
}  // namespace SamplingMenu
