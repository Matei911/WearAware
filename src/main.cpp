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
   bool wasConnected = false;
   uint32_t updateIntervalMs = 0;
   unsigned long lastPublishMs = 0;
   SensorReadings uiReadings;
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

void sleepAppModeHardware()
{
   DisplayManager::sleep();
   Sensors::sleep();
}

void runDeviceCycle(bool showWaitingScreen,
                    const SensorReadings& transitionReadings)
{
   if (showWaitingScreen)
   {
      DisplayManager::renderWaitingForDataScreen(transitionReadings);
   }

   const SensorReadings readings = Sensors::readAll();
   DisplayManager::renderHomeScreen(readings);
   DisplayManager::sleep();
   Sensors::sleep();
   PowerManager::enterDeepSleep(
       ModeSettings::selectedDeviceSleepDurationUs());
}

bool startAppMode(const SensorReadings& menuReadings)
{
   if (!BleEnvironment::start())
   {
      sleepAppModeHardware();
      return false;
   }

   appModeState.active = true;
   appModeState.wasConnected = BleEnvironment::isConnected();
   appModeState.updateIntervalMs =
       ModeSettings::selectedAppUpdateIntervalMs();
   appModeState.lastPublishMs = 0;
   appModeState.uiReadings = menuReadings;

   DisplayManager::renderPhonePromptScreen(appModeState.uiReadings);
   sleepAppModeHardware();
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
         if (startAppMode(menuReadings))
         {
            return;
         }

         continue;
      }

      runDeviceCycle(true, menuReadings);
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

   const bool connected = BleEnvironment::isConnected();

   if (connected && !appModeState.wasConnected)
   {
      const unsigned long connectedAtMs = millis();
      DisplayManager::renderConnectedPromptScreen(appModeState.uiReadings);

      const SensorReadings readings = Sensors::readAll();
      BleEnvironment::publish(readings);
      appModeState.uiReadings = readings;
      appModeState.lastPublishMs = connectedAtMs;
      appModeState.wasConnected = true;
      sleepAppModeHardware();
      delay(20);
      return;
   }

   if (!connected && appModeState.wasConnected)
   {
      appModeState.wasConnected = false;
      DisplayManager::renderPhonePromptScreen(appModeState.uiReadings);
      sleepAppModeHardware();
      delay(20);
      return;
   }

   if (!connected)
   {
      sleepAppModeHardware();
      delay(20);
      return;
   }

   const unsigned long now = millis();
   if (now - appModeState.lastPublishMs < appModeState.updateIntervalMs)
   {
      sleepAppModeHardware();
      delay(20);
      return;
   }

   appModeState.lastPublishMs = now;

   const SensorReadings readings = Sensors::readAll();
   BleEnvironment::publish(readings);
   appModeState.uiReadings = readings;
   sleepAppModeHardware();
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

   runDeviceCycle(false, SensorReadings());
}

void loop()
{
   tickAppMode();
}
