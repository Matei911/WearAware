#include "DisplayScreens.h"

#include <Fonts/FreeMonoBold9pt7b.h>

#include "BleSettings.h"
#include "SamplingMenu.h"

namespace
{
struct MenuOptionLayout
{
   SamplingMenu::MenuRow row;
   int boxY;
};

constexpr MenuOptionLayout OPTION_LAYOUTS[] = {
    {SamplingMenu::MenuRow::FiveMinutes, 58},
    {SamplingMenu::MenuRow::OneMinute, 87},
    {SamplingMenu::MenuRow::ThirtySeconds, 116},
    {SamplingMenu::MenuRow::BleToggle, 145},
};

constexpr int MENU_BOX_X = 30;
constexpr int MENU_BOX_WIDTH = 140;
constexpr int MENU_BOX_HEIGHT = 22;
constexpr int MENU_BOX_RADIUS = 10;

uint8_t optionIndex(SamplingMenu::MenuRow row)
{
   return static_cast<uint8_t>(row);
}

const char* optionLabel(SamplingMenu::MenuRow row)
{
   switch (row)
   {
      case SamplingMenu::MenuRow::FiveMinutes:
         return "5 minutes";
      case SamplingMenu::MenuRow::OneMinute:
         return "1 minute";
      case SamplingMenu::MenuRow::ThirtySeconds:
         return "30 seconds";
      case SamplingMenu::MenuRow::BleToggle:
      default:
         return BleSettings::isEnabled() ? "BLE: ON" : "BLE: OFF";
   }
}

void drawCenteredOption(WearAwareDisplay& display,
                        SamplingMenu::MenuRow row)
{
   const char* label = optionLabel(row);
   const int boxY = OPTION_LAYOUTS[optionIndex(row)].boxY;
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
}  // namespace

namespace SamplingMenuScreen
{
void draw(WearAwareDisplay& display,
          SamplingInterval /*selectedInterval*/,
          const SensorReadings& readings)
{
   const SamplingMenu::MenuRow currentRow = SamplingMenu::selectedRow();
   const MenuOptionLayout& selectedLayout =
       OPTION_LAYOUTS[optionIndex(currentRow)];

   display.fillScreen(GxEPD_WHITE);
   display.setTextWrap(false);
   ScreenHeader::draw(display, readings.batteryPercent);

   display.setCursor(29, 54);
   display.print("Sampling menu");

   for (const MenuOptionLayout& option : OPTION_LAYOUTS)
   {
      drawCenteredOption(display, option.row);
   }

   display.drawRoundRect(MENU_BOX_X,
                         selectedLayout.boxY,
                         MENU_BOX_WIDTH,
                         MENU_BOX_HEIGHT,
                         MENU_BOX_RADIUS,
                         GxEPD_BLACK);
}
}  // namespace SamplingMenuScreen
