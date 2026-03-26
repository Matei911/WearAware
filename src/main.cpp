#include <Arduino.h>

#include "BleEnvironment.h"
#include "BleSettings.h"
#include "DisplayManager.h"
#include "PowerManager.h"
#include "SamplingMenu.h"
#include "SamplingSettings.h"
#include "Sensors.h"

namespace
{
void runMenuFlow()
{
   SensorReadings menuReadings;
   menuReadings.batteryPercent = Sensors::batteryPercentForMenu();

   DisplayManager::init();

   while (true)
   {
      const SamplingMenu::MenuSelection selection =
          SamplingMenu::run(menuReadings);

      if (selection.mode == SamplingMenu::ModeRow::ConnectToApp)
      {
         BleSettings::setEnabled(true);
         BleSettings::setUpdateInterval(selection.bleUpdateInterval);

         Sensors::init();
         const SensorReadings initialReadings = Sensors::readAll();
         menuReadings = BleEnvironment::run(initialReadings);
         menuReadings.batteryPercent = Sensors::batteryPercentForMenu();
         DisplayManager::forceNextFullRefresh();
         continue;
      }

      SamplingSettings::setSelectedInterval(selection.deepSleepInterval);
      BleSettings::setEnabled(false);
      DisplayManager::renderWaitingForDataScreen(menuReadings);
      return;
   }
}
}  // namespace

void setup()
{
   Serial.begin(115200);
   delay(200);

   PowerManager::init();
   BleSettings::init();
   SamplingSettings::init();

   if (PowerManager::shouldEnterSamplingMenu())
   {
      runMenuFlow();
   }

   Sensors::init();
   const SensorReadings readings = Sensors::readAll();

   BleSettings::setEnabled(false);
   DisplayManager::init();
   DisplayManager::renderHomeScreen(readings);

   PowerManager::enterDeepSleep(
       SamplingSettings::selectedSleepDurationUs());
}

void loop() {}
