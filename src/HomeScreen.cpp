#include "DisplayScreens.h"

#include <Fonts/FreeMonoBold9pt7b.h>
#include <stdio.h>

namespace
{
String formatTimeString(const SensorReadings& readings)
{
   unsigned int hours = 0;
   unsigned int minutes = 0;

   if (sscanf(readings.timestamp,
              "%*u-%*u-%*u %u:%u:%*u",
              &hours,
              &minutes) != 2)
   {
      return "--:--";
   }

   char formattedTime[6];
   snprintf(formattedTime,
            sizeof(formattedTime),
            "%02u:%02u",
            hours,
            minutes);
   return String(formattedTime);
}
}  // namespace

namespace HomeScreen
{
void draw(WearAwareDisplay& display, const SensorReadings& readings)
{
   display.fillScreen(GxEPD_WHITE);
   // display.fillRect(0,0,200,200, GxEPD_WHITE);
   // display.setTextColor(GxEPD_BLACK, GxEPD_WHITE);
   ScreenHeader::draw(display, readings.batteryPercent);

   display.setCursor(7, 44);
   display.print("PM1:   " + String(readings.pm1));

   display.setCursor(7, 64);
   display.print("PM2.5: " + String(readings.pm25));

   display.setCursor(7, 84);
   display.print("PM10:  " + String(readings.pm10));

   display.setCursor(7, 104);
   display.print("CO2:   " + String(readings.co2ppm) + " ppm");

   display.setCursor(7, 124);
   display.print("Tmp:   " + String(readings.temperature));

   display.setCursor(7, 144);
   display.print("Hu:    " + String(readings.humidity));

   display.setCursor(7, 164);
   display.print("Press: " + String(readings.pressure) + " hPa");

   display.setCursor(7, 193);
   display.print("Last synced " + formatTimeString(readings));
}
}  // namespace HomeScreen
