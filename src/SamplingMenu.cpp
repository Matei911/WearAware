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
constexpr uint8_t MENU_ROW_COUNT = 4;

SamplingMenu::MenuRow currentRow = SamplingMenu::MenuRow::FiveMinutes;

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

SamplingMenu::MenuRow rowFromInterval(SamplingInterval interval)
{
   switch (interval)
   {
      case SamplingInterval::FiveMinutes:
         return SamplingMenu::MenuRow::FiveMinutes;
      case SamplingInterval::OneMinute:
         return SamplingMenu::MenuRow::OneMinute;
      case SamplingInterval::ThirtySeconds:
      default:
         return SamplingMenu::MenuRow::ThirtySeconds;
   }
}

SamplingInterval intervalFromRow(SamplingMenu::MenuRow row)
{
   switch (row)
   {
      case SamplingMenu::MenuRow::FiveMinutes:
         return SamplingInterval::FiveMinutes;
      case SamplingMenu::MenuRow::OneMinute:
         return SamplingInterval::OneMinute;
      case SamplingMenu::MenuRow::ThirtySeconds:
      default:
         return SamplingInterval::ThirtySeconds;
   }
}

SamplingMenu::MenuRow nextRow(SamplingMenu::MenuRow row)
{
   const uint8_t rowIndex = static_cast<uint8_t>(row);
   return static_cast<SamplingMenu::MenuRow>(
       (rowIndex + 1) % MENU_ROW_COUNT);
}

SamplingMenu::MenuRow previousRow(SamplingMenu::MenuRow row)
{
   const uint8_t rowIndex = static_cast<uint8_t>(row);
   return static_cast<SamplingMenu::MenuRow>(
       (rowIndex + MENU_ROW_COUNT - 1) % MENU_ROW_COUNT);
}

void applySelectedRow(SamplingInterval& selectedInterval)
{
   if (currentRow == SamplingMenu::MenuRow::BleToggle)
   {
      BleSettings::toggle();
      return;
   }

   selectedInterval = intervalFromRow(currentRow);
   SamplingSettings::setSelectedInterval(selectedInterval);
}
}  // namespace

namespace SamplingMenu
{
MenuRow selectedRow()
{
   return currentRow;
}

void run(const SensorReadings& readings)
{
   SamplingInterval selectedInterval =
       SamplingSettings::getSelectedInterval();
   currentRow = rowFromInterval(selectedInterval);

   DisplayManager::renderSamplingMenu(selectedInterval, false, readings);
   PowerManager::waitForButtonsReleased();
   delay(BUTTON_DEBOUNCE_MS);

   while (true)
   {
      if (buttonPressed(AppConfig::BUTTON_PIN_2))
      {
         currentRow = previousRow(currentRow);
         DisplayManager::renderSamplingMenu(selectedInterval, true, readings);
         waitForButtonRelease(AppConfig::BUTTON_PIN_2);
         continue;
      }

      if (buttonPressed(AppConfig::BUTTON_PIN_3))
      {
         currentRow = nextRow(currentRow);
         DisplayManager::renderSamplingMenu(selectedInterval, true, readings);
         waitForButtonRelease(AppConfig::BUTTON_PIN_3);
         continue;
      }

      if (buttonPressed(AppConfig::BUTTON_PIN_1))
      {
         applySelectedRow(selectedInterval);
         DisplayManager::forceNextFullRefresh();
         waitForButtonRelease(AppConfig::BUTTON_PIN_1);
         return;
      }

      delay(20);
   }
}
}  // namespace SamplingMenu
