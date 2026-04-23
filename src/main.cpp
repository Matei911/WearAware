#include <Arduino.h>
#include <stdarg.h>
#include <stdio.h>

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
   bool hardwareSleeping = false;
   uint32_t updateIntervalMs = 0;
   unsigned long lastPublishMs = 0;
   SensorReadings uiReadings;
};

AppModeState appModeState;

void sleepAppModeHardware();

#if WEARAWARE_ENABLE_SENSOR_DEBUG_LOGS
void debugAppLog(const char* message)
{
   Serial.printf("[app %lu ms] %s\n",
                 static_cast<unsigned long>(millis()),
                 message);
}

void debugAppLogf(const char* format, ...)
{
   char message[180];
   va_list args;
   va_start(args, format);
   vsnprintf(message, sizeof(message), format, args);
   va_end(args);

   debugAppLog(message);
}
#else
void debugAppLog(const char* /*message*/) {}
void debugAppLogf(const char* /*format*/, ...) {}
#endif

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
   sleepAppModeHardware();
   BleEnvironment::stop();
   appModeState = AppModeState();
}

void sleepAppModeHardware()
{
   if (appModeState.hardwareSleeping)
   {
      return;
   }

   DisplayManager::sleep();
   Sensors::sleep();
   appModeState.hardwareSleeping = true;
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
      debugAppLog("app mode BLE start failed");
      sleepAppModeHardware();
      return false;
   }

   appModeState.active = true;
   appModeState.wasConnected = BleEnvironment::isConnected();
   appModeState.updateIntervalMs =
       ModeSettings::selectedAppUpdateIntervalMs();
   appModeState.lastPublishMs = 0;
   appModeState.uiReadings = menuReadings;

   debugAppLogf("app mode started: interval=%lu ms connected=%s",
                static_cast<unsigned long>(
                    appModeState.updateIntervalMs),
                appModeState.wasConnected ? "yes" : "no");

   DisplayManager::renderPhonePromptScreen(appModeState.uiReadings);
   appModeState.hardwareSleeping = false;
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
      debugAppLog("app mode exit requested by B1");
      DisplayManager::renderReturningToMenuScreen(
          appModeState.uiReadings);
      appModeState.hardwareSleeping = false;
      waitForButtonRelease(AppConfig::BUTTON_PIN_1);
      stopAppMode();
      runModeMenu();
      return;
   }

   const bool connected = BleEnvironment::isConnected();

   if (connected && !appModeState.wasConnected)
   {
      const unsigned long connectedAtMs = millis();
      debugAppLog("BLE connected; first app-mode sensor read starting");
      DisplayManager::renderConnectedPromptScreen(appModeState.uiReadings);
      appModeState.hardwareSleeping = false;

      const SensorReadings readings = Sensors::readAll();
      appModeState.hardwareSleeping = false;
      debugAppLog("first app-mode publish start");
      BleEnvironment::publish(readings);
      debugAppLog("first app-mode publish done");
      appModeState.uiReadings = readings;
      appModeState.lastPublishMs = connectedAtMs;
      appModeState.wasConnected = true;
      sleepAppModeHardware();
      delay(AppConfig::APP_MODE_SETTLE_DELAY_MS);
      return;
   }

   if (!connected && appModeState.wasConnected)
   {
      appModeState.wasConnected = false;
      DisplayManager::renderPhonePromptScreen(appModeState.uiReadings);
      appModeState.hardwareSleeping = false;
      sleepAppModeHardware();
      delay(AppConfig::APP_MODE_SETTLE_DELAY_MS);
      return;
   }

   if (!connected)
   {
      sleepAppModeHardware();
      delay(AppConfig::APP_MODE_ADVERTISING_IDLE_DELAY_MS);
      return;
   }

   const unsigned long now = millis();
   const unsigned long elapsedSincePublish =
       now - appModeState.lastPublishMs;
   if (elapsedSincePublish < appModeState.updateIntervalMs)
   {
      sleepAppModeHardware();
      delay(AppConfig::APP_MODE_CONNECTED_IDLE_DELAY_MS);
      return;
   }

   appModeState.lastPublishMs = now;
   debugAppLogf("periodic app-mode read due: elapsed=%lu ms "
                "interval=%lu ms",
                static_cast<unsigned long>(elapsedSincePublish),
                static_cast<unsigned long>(
                    appModeState.updateIntervalMs));

   const SensorReadings readings = Sensors::readAll();
   appModeState.hardwareSleeping = false;
   debugAppLog("periodic app-mode publish start");
   BleEnvironment::publish(readings);
   debugAppLog("periodic app-mode publish done");
   appModeState.uiReadings = readings;
   sleepAppModeHardware();
   delay(AppConfig::APP_MODE_SETTLE_DELAY_MS);
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
