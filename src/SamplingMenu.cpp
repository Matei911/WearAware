#include "SamplingMenu.h"

#include <Arduino.h>

#include "AppConfig.h"
#include "BleSettings.h"
#include "DisplayManager.h"
#include "PowerManager.h"
#include "SamplingSettings.h"

namespace
{
constexpr uint16_t BUTTON_DEBOUNCE_MS = 300;
constexpr uint8_t MODE_ROW_COUNT = 2;
constexpr uint8_t DEEP_SLEEP_ROW_COUNT = 3;
constexpr uint8_t APP_DURATION_ROW_COUNT = 3;

enum class MenuState : uint8_t
{
   ChooseMode = 0,
   DeepSleepDuration = 1,
   AppConnectionDuration = 2
};

MenuState currentState = MenuState::ChooseMode;
SamplingMenu::ModeRow currentModeRow = SamplingMenu::ModeRow::ConnectToApp;
SamplingMenu::DeepSleepRow currentDeepSleepRow =
    SamplingMenu::DeepSleepRow::FiveMinutes;
SamplingMenu::AppDurationRow currentAppDurationRow =
    SamplingMenu::AppDurationRow::SixtySeconds;

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

SamplingMenu::DeepSleepRow rowFromInterval(SamplingInterval interval)
{
   switch (interval)
   {
      case SamplingInterval::FiveMinutes:
         return SamplingMenu::DeepSleepRow::FiveMinutes;
      case SamplingInterval::ThreeMinutes:
         return SamplingMenu::DeepSleepRow::ThreeMinutes;
      case SamplingInterval::OneMinute:
      default:
         return SamplingMenu::DeepSleepRow::OneMinute;
   }
}

SamplingInterval intervalFromRow(SamplingMenu::DeepSleepRow row)
{
   switch (row)
   {
      case SamplingMenu::DeepSleepRow::FiveMinutes:
         return SamplingInterval::FiveMinutes;
      case SamplingMenu::DeepSleepRow::ThreeMinutes:
         return SamplingInterval::ThreeMinutes;
      case SamplingMenu::DeepSleepRow::OneMinute:
      default:
         return SamplingInterval::OneMinute;
   }
}

SamplingMenu::AppDurationRow rowFromUpdateInterval(
    BleSettings::UpdateInterval interval)
{
   switch (interval)
   {
      case BleSettings::UpdateInterval::SixtySeconds:
         return SamplingMenu::AppDurationRow::SixtySeconds;
      case BleSettings::UpdateInterval::ThirtySeconds:
         return SamplingMenu::AppDurationRow::ThirtySeconds;
      case BleSettings::UpdateInterval::FifteenSeconds:
      default:
         return SamplingMenu::AppDurationRow::FifteenSeconds;
   }
}

BleSettings::UpdateInterval updateIntervalFromRow(
    SamplingMenu::AppDurationRow row)
{
   switch (row)
   {
      case SamplingMenu::AppDurationRow::SixtySeconds:
         return BleSettings::UpdateInterval::SixtySeconds;
      case SamplingMenu::AppDurationRow::ThirtySeconds:
         return BleSettings::UpdateInterval::ThirtySeconds;
      case SamplingMenu::AppDurationRow::FifteenSeconds:
      default:
         return BleSettings::UpdateInterval::FifteenSeconds;
   }
}

SamplingMenu::MenuSelection makeDeepSleepSelection(
    SamplingMenu::DeepSleepRow row)
{
   SamplingMenu::MenuSelection selection;
   selection.mode = SamplingMenu::ModeRow::DeepSleepSample;
   selection.deepSleepInterval = intervalFromRow(row);
   selection.bleUpdateInterval = BleSettings::getUpdateInterval();
   return selection;
}

SamplingMenu::MenuSelection makeAppConnectSelection(
    SamplingMenu::AppDurationRow row)
{
   SamplingMenu::MenuSelection selection;
   selection.mode = SamplingMenu::ModeRow::ConnectToApp;
   selection.deepSleepInterval =
       SamplingSettings::getSelectedInterval();
   selection.bleUpdateInterval = updateIntervalFromRow(row);
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
      case MenuState::DeepSleepDuration:
         DisplayManager::renderDeepSleepMenu(currentDeepSleepRow,
                                             usePartialRefresh,
                                             readings);
         return;
      case MenuState::AppConnectionDuration:
         DisplayManager::renderAppConnectMenu(currentAppDurationRow,
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
   currentModeRow = ModeRow::ConnectToApp;
   currentDeepSleepRow =
       rowFromInterval(SamplingSettings::getSelectedInterval());
   currentAppDurationRow =
       rowFromUpdateInterval(BleSettings::getUpdateInterval());

   BleSettings::setEnabled(false);
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
            case MenuState::DeepSleepDuration:
               currentDeepSleepRow = previousRow(currentDeepSleepRow,
                                                 DEEP_SLEEP_ROW_COUNT);
               renderCurrentScreen(true, readings);
               break;
            case MenuState::AppConnectionDuration:
               currentAppDurationRow = previousRow(currentAppDurationRow,
                                                   APP_DURATION_ROW_COUNT);
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
            case MenuState::DeepSleepDuration:
               currentDeepSleepRow = nextRow(currentDeepSleepRow,
                                             DEEP_SLEEP_ROW_COUNT);
               renderCurrentScreen(true, readings);
               break;
            case MenuState::AppConnectionDuration:
               currentAppDurationRow = nextRow(currentAppDurationRow,
                                               APP_DURATION_ROW_COUNT);
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
               if (currentModeRow == ModeRow::ConnectToApp)
               {
                  BleSettings::setEnabled(true);
                  currentState = MenuState::AppConnectionDuration;
               }
               else
               {
                  BleSettings::setEnabled(false);
                  currentState = MenuState::DeepSleepDuration;
               }

               DisplayManager::forceNextFullRefresh();
               renderCurrentScreen(false, readings);
               break;
            case MenuState::DeepSleepDuration:
               BleSettings::setEnabled(false);
               DisplayManager::forceNextFullRefresh();
               return makeDeepSleepSelection(currentDeepSleepRow);
            case MenuState::AppConnectionDuration:
               DisplayManager::forceNextFullRefresh();
               return makeAppConnectSelection(currentAppDurationRow);
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
