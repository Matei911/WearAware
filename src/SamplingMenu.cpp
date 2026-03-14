#include "SamplingMenu.h"

#include <Arduino.h>

#include "AppConfig.h"
#include "DisplayManager.h"
#include "PowerManager.h"
#include "SamplingSettings.h"

namespace
{
constexpr uint16_t BUTTON_DEBOUNCE_MS = 300;

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
}  // namespace

namespace SamplingMenu
{
void run(const SensorReadings& readings)
{
   SamplingInterval selectedInterval =
       SamplingSettings::getSelectedInterval();

   DisplayManager::renderSamplingMenu(selectedInterval, false, readings);
   PowerManager::waitForButtonsReleased();
   delay(BUTTON_DEBOUNCE_MS);

   while (true)
   {
      if (buttonPressed(AppConfig::BUTTON_PIN_2))
      {
         selectedInterval =
             SamplingSettings::previousInterval(selectedInterval);
         DisplayManager::renderSamplingMenu(selectedInterval, true, readings);
         waitForButtonRelease(AppConfig::BUTTON_PIN_2);
         continue;
      }

      if (buttonPressed(AppConfig::BUTTON_PIN_3))
      {
         selectedInterval =
             SamplingSettings::nextInterval(selectedInterval);
         DisplayManager::renderSamplingMenu(selectedInterval, true, readings);
         waitForButtonRelease(AppConfig::BUTTON_PIN_3);
         continue;
      }

      if (buttonPressed(AppConfig::BUTTON_PIN_1))
      {
         SamplingSettings::setSelectedInterval(selectedInterval);
         DisplayManager::forceNextFullRefresh();
         waitForButtonRelease(AppConfig::BUTTON_PIN_1);
         return;
      }

      delay(20);
   }
}
}  // namespace SamplingMenu
