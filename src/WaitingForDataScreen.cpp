#include "DisplayScreens.h"

#include <Fonts/FreeMonoBold9pt7b.h>

namespace WaitingForDataScreen
{
void draw(WearAwareDisplay& display, const SensorReadings& readings)
{
   display.fillScreen(GxEPD_WHITE);
   display.setTextWrap(false);
   ScreenHeader::draw(display, readings.batteryPercent);

   display.setCursor(30, 95);
   display.print("Waiting to");

   display.setCursor(22, 123);
   display.print("read data...");
}
}  // namespace WaitingForDataScreen
