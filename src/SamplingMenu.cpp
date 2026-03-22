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
   AppConnectionDuration = 2,
   PhonePrompt = 3
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
      case MenuState::PhonePrompt:
      default:
         DisplayManager::renderPhonePromptScreen(readings);
         return;
   }
}
}  // namespace

namespace SamplingMenu
{
void run(const SensorReadings& readings)
{
   currentState = MenuState::ChooseMode;
   currentModeRow = ModeRow::ConnectToApp;
   currentDeepSleepRow =
       rowFromInterval(SamplingSettings::getSelectedInterval());
   currentAppDurationRow = AppDurationRow::SixtySeconds;

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
            case MenuState::PhonePrompt:
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
            case MenuState::PhonePrompt:
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
               SamplingSettings::setSelectedInterval(
                   intervalFromRow(currentDeepSleepRow));
               BleSettings::setEnabled(false);
               DisplayManager::forceNextFullRefresh();
               return;
            case MenuState::AppConnectionDuration:
               currentState = MenuState::PhonePrompt;
               DisplayManager::forceNextFullRefresh();
               renderCurrentScreen(false, readings);
               break;
            case MenuState::PhonePrompt:
            default:
               BleSettings::setEnabled(false);
               currentState = MenuState::ChooseMode;
               DisplayManager::forceNextFullRefresh();
               renderCurrentScreen(false, readings);
               break;
         }

         continue;
      }

      delay(20);
   }
}
}  // namespace SamplingMenu
