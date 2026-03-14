#include <Arduino.h>

#include "DisplayManager.h"
#include "PowerManager.h"
#include "SamplingMenu.h"
#include "SamplingSettings.h"
#include "Sensors.h"

void setup()
{
   Serial.begin(115200);
   delay(200);

   PowerManager::init();
   SamplingSettings::init();

   if (PowerManager::shouldEnterSamplingMenu())
   {
      SensorReadings menuReadings;
      menuReadings.batteryPercent = Sensors::batteryPercentForMenu();
      DisplayManager::init();
      SamplingMenu::run(menuReadings);
      DisplayManager::renderWaitingForDataScreen(menuReadings);
   }

   Sensors::init();
   const SensorReadings readings = Sensors::readAll();

   DisplayManager::init();
   DisplayManager::renderHomeScreen(readings);

   PowerManager::enterDeepSleep(
       SamplingSettings::selectedSleepDurationUs());
}

void loop() {}
