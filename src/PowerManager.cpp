#include "PowerManager.h"

#include <Arduino.h>
#include <esp_sleep.h>

#include "AppConfig.h"

namespace PowerManager
{
void init()
{
   pinMode(AppConfig::BUTTON_PIN_1, INPUT_PULLUP);
   pinMode(AppConfig::BUTTON_PIN_2, INPUT_PULLUP);
   pinMode(AppConfig::BUTTON_PIN_3, INPUT_PULLUP);
}

bool shouldEnterSamplingMenu()
{
   const esp_sleep_wakeup_cause_t wakeCause =
       esp_sleep_get_wakeup_cause();

   if (wakeCause == ESP_SLEEP_WAKEUP_EXT1)
   {
      return (esp_sleep_get_ext1_wakeup_status() &
              (1ULL << AppConfig::BUTTON_PIN_1)) != 0;
   }

   return digitalRead(AppConfig::BUTTON_PIN_1) == LOW;
}

void waitForButtonsReleased()
{
   while (digitalRead(AppConfig::BUTTON_PIN_1) == LOW ||
          digitalRead(AppConfig::BUTTON_PIN_2) == LOW ||
          digitalRead(AppConfig::BUTTON_PIN_3) == LOW)
   {
      delay(10);
   }
}

void enterDeepSleep(uint64_t sleepDurationUs)
{
   waitForButtonsReleased();

   esp_sleep_enable_timer_wakeup(sleepDurationUs);
   esp_sleep_enable_ext1_wakeup(
       (1ULL << AppConfig::BUTTON_PIN_1),
       ESP_EXT1_WAKEUP_ANY_LOW);

   esp_deep_sleep_start();
}
}  // namespace PowerManager
