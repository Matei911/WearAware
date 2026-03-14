#include "DisplayScreens.h"

#include <Fonts/FreeMonoBold9pt7b.h>

namespace
{
struct MenuOptionLayout
{
   const char* label;
   int cursorX;
   int cursorY;
   int boxY;
};

constexpr MenuOptionLayout OPTION_LAYOUTS[] = {
    {"5 minutes", 51, 89, 72},
    {"1 minute", 56, 124, 107},
    {"30 seconds", 45, 159, 142},
};

uint8_t optionIndex(SamplingInterval interval)
{
   return static_cast<uint8_t>(interval);
}
}  // namespace

namespace SamplingMenuScreen
{
void draw(WearAwareDisplay& display,
          SamplingInterval selectedInterval,
          const SensorReadings& readings)
{
   const MenuOptionLayout& selectedLayout =
       OPTION_LAYOUTS[optionIndex(selectedInterval)];

   display.fillScreen(GxEPD_WHITE);
   display.setTextWrap(false);
   ScreenHeader::draw(display, readings.batteryPercent);

   display.setCursor(29, 54);
   display.print("Sampling menu");

   display.setCursor(OPTION_LAYOUTS[0].cursorX, OPTION_LAYOUTS[0].cursorY);
   display.print(OPTION_LAYOUTS[0].label);

   display.setCursor(OPTION_LAYOUTS[1].cursorX, OPTION_LAYOUTS[1].cursorY);
   display.print(OPTION_LAYOUTS[1].label);

   display.setCursor(OPTION_LAYOUTS[2].cursorX, OPTION_LAYOUTS[2].cursorY);
   display.print(OPTION_LAYOUTS[2].label);

   display.drawRoundRect(40,
                         selectedLayout.boxY,
                         120,
                         25,
                         10,
                         GxEPD_BLACK);
}
}  // namespace SamplingMenuScreen
