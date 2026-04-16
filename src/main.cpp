#include <Arduino.h>

#include "AppConfig.h"
#include "BleEnvironment.h"
#include "DisplayManager.h"
#include "ModeSettings.h"
#include "PowerManager.h"
#include "SamplingMenu.h"
#include "Sensors.h"

namespace
{
constexpr uint16_t BUTTON_DEBOUNCE_MS = 300;

struct AppModeState
{
   bool active = false;
   uint32_t updateIntervalMs = 0;
   unsigned long lastPublishMs = 0;
};

AppModeState appModeState;

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

void stopAppMode()
{
   BleEnvironment::stop();
   appModeState = AppModeState();
}

void runDeviceCycle()
{
   const SensorReadings readings = Sensors::readAll();
   DisplayManager::renderHomeScreen(readings);
   DisplayManager::sleep();
   Sensors::sleep();
   PowerManager::enterDeepSleep(
       ModeSettings::selectedDeviceSleepDurationUs());
}

bool startAppMode()
{
   const SensorReadings initialReadings = Sensors::readAll();

   if (!BleEnvironment::start(initialReadings))
   {
      Sensors::sleep();
      return false;
   }

   DisplayManager::renderPhonePromptScreen(initialReadings);
   DisplayManager::sleep();
   Sensors::sleep();

   appModeState.active = true;
   appModeState.updateIntervalMs =
       ModeSettings::selectedAppUpdateIntervalMs();
   appModeState.lastPublishMs = millis();
   return true;
}

void runModeMenu()
{
   while (true)
   {
      SensorReadings menuReadings;
      menuReadings.batteryPercent = Sensors::batteryPercentForMenu();
      Sensors::sleep();

      const SamplingMenu::MenuSelection selection =
          SamplingMenu::run(menuReadings);

      ModeSettings::setDeviceInterval(selection.deviceInterval);
      ModeSettings::setAppInterval(selection.appInterval);

      if (selection.mode == SamplingMenu::ModeRow::AppMode)
      {
         if (startAppMode())
         {
            return;
         }

         continue;
      }

      runDeviceCycle();
   }
}

void tickAppMode()
{
   if (!appModeState.active)
   {
      return;
   }

   if (buttonPressed(AppConfig::BUTTON_PIN_1))
   {
      waitForButtonRelease(AppConfig::BUTTON_PIN_1);
      stopAppMode();
      runModeMenu();
      return;
   }

   const unsigned long now = millis();
   if (now - appModeState.lastPublishMs < appModeState.updateIntervalMs)
   {
      delay(20);
      return;
   }

   appModeState.lastPublishMs = now;

   const SensorReadings readings = Sensors::readAll();
   BleEnvironment::publish(readings);
   Sensors::sleep();
   delay(20);
}
}  // namespace

void setup()
{
   Serial.begin(115200);
   delay(200);

   PowerManager::init();
   ModeSettings::init();

   if (PowerManager::shouldEnterSamplingMenu())
   {
      runModeMenu();
      return;
   }

   runDeviceCycle();
}

void loop()
{
   tickAppMode();
}
