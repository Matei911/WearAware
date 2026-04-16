#include "DisplayScreens.h"

#include <Fonts/FreeMonoBold9pt7b.h>

namespace
{
void drawCenteredLine(WearAwareDisplay& display,
                      const char* text,
                      int baselineY)
{
   int16_t x1 = 0;
   int16_t y1 = 0;
   uint16_t width = 0;
   uint16_t height = 0;

   display.getTextBounds(text, 0, 0, &x1, &y1, &width, &height);
   display.setCursor((display.width() - width) / 2 - x1, baselineY);
   display.print(text);
}
}  // namespace

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

namespace PhonePromptScreen
{
void draw(WearAwareDisplay& display, const SensorReadings& readings)
{
   display.fillScreen(GxEPD_WHITE);
   display.setTextWrap(false);
   ScreenHeader::draw(display, readings.batteryPercent);

   drawCenteredLine(display, "Check the phone", 95);
   drawCenteredLine(display, "B1: go back", 123);
}
}  // namespace PhonePromptScreen

namespace ConnectedPromptScreen
{
void draw(WearAwareDisplay& display, const SensorReadings& readings)
{
   display.fillScreen(GxEPD_WHITE);
   display.setTextWrap(false);
   ScreenHeader::draw(display, readings.batteryPercent);

   drawCenteredLine(display, "BLE connected", 95);
   drawCenteredLine(display, "Starting sync...", 123);
}
}  // namespace ConnectedPromptScreen
