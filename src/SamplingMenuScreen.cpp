#include "DisplayScreens.h"

#include <Fonts/FreeMonoBold9pt7b.h>

namespace
{
constexpr int MENU_BOX_X = 10;
constexpr int MENU_BOX_WIDTH = 180;
constexpr int MENU_BOX_HEIGHT = 22;
constexpr int MENU_BOX_RADIUS = 10;
constexpr int TITLE_BASELINE_Y = 50;

constexpr int MODE_OPTION_BOX_Y[] = {78, 107};
constexpr int THREE_OPTION_BOX_Y[] = {68, 97, 126};

void drawCenteredText(WearAwareDisplay& display,
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

void drawCenteredOption(WearAwareDisplay& display,
                        const char* label,
                        int boxY)
{
   int16_t x1 = 0;
   int16_t y1 = 0;
   uint16_t width = 0;
   uint16_t height = 0;

   display.getTextBounds(label,
                         0,
                         0,
                         &x1,
                         &y1,
                         &width,
                         &height);

   const int cursorX =
       MENU_BOX_X + ((MENU_BOX_WIDTH - width) / 2) - x1;
   const int cursorY =
       boxY + ((MENU_BOX_HEIGHT - height) / 2) - y1;

   display.setCursor(cursorX, cursorY);
   display.print(label);
}

void drawMenu(WearAwareDisplay& display,
              const char* title,
              const char* const* labels,
              const int* boxYPositions,
              uint8_t optionCount,
              uint8_t selectedIndex,
              const SensorReadings& readings)
{
   display.fillScreen(GxEPD_WHITE);
   display.setTextWrap(false);
   ScreenHeader::draw(display, readings.batteryPercent);

   drawCenteredText(display, title, TITLE_BASELINE_Y);

   for (uint8_t i = 0; i < optionCount; ++i)
   {
      drawCenteredOption(display, labels[i], boxYPositions[i]);
   }

   display.drawRoundRect(MENU_BOX_X,
                         boxYPositions[selectedIndex],
                         MENU_BOX_WIDTH,
                         MENU_BOX_HEIGHT,
                         MENU_BOX_RADIUS,
                         GxEPD_BLACK);
}
}  // namespace

namespace SamplingMenuScreen
{
void drawModeMenu(WearAwareDisplay& display,
                  SamplingMenu::ModeRow selectedRow,
                  const SensorReadings& readings)
{
   const char* labels[] = {
       "Device Mode",
       "App Mode",
   };

   drawMenu(display,
            "Choose mode",
            labels,
            MODE_OPTION_BOX_Y,
            sizeof(labels) / sizeof(labels[0]),
            static_cast<uint8_t>(selectedRow),
            readings);
}

void drawDeviceIntervalMenu(
    WearAwareDisplay& display,
    SamplingMenu::DeviceIntervalRow selectedRow,
    const SensorReadings& readings)
{
   const char* labels[] = {
       "5 minutes",
       "3 minutes",
       "1 minute",
   };

   drawMenu(display,
            "Device Mode",
            labels,
            THREE_OPTION_BOX_Y,
            sizeof(labels) / sizeof(labels[0]),
            static_cast<uint8_t>(selectedRow),
            readings);
}

void drawAppIntervalMenu(WearAwareDisplay& display,
                         SamplingMenu::AppIntervalRow selectedRow,
                         const SensorReadings& readings)
{
   const char* labels[] = {
       "60 seconds",
       "30 seconds",
       "15 seconds",
   };

   drawMenu(display,
            "App Mode",
            labels,
            THREE_OPTION_BOX_Y,
            sizeof(labels) / sizeof(labels[0]),
            static_cast<uint8_t>(selectedRow),
            readings);
}
}  // namespace SamplingMenuScreen
