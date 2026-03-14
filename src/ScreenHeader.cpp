#include "DisplayScreens.h"

#include <Fonts/FreeMonoBold9pt7b.h>

namespace
{
int batteryFillWidth(float batteryPercent)
{
   const int fill =
       static_cast<int>((batteryPercent / 100.0f) * 19.0f);

   return constrain(fill, 0, 19);
}
}  // namespace

namespace ScreenHeader
{
void draw(WearAwareDisplay& display, float batteryPercent)
{
   display.setTextColor(GxEPD_BLACK);
   display.setFont(&FreeMonoBold9pt7b);

   display.setCursor(51, 19);
   display.print("WearAware");

   display.drawRect(173, 5, 21, 10, GxEPD_BLACK);
   display.fillRect(175,
                    7,
                    batteryFillWidth(batteryPercent),
                    6,
                    GxEPD_BLACK);
   display.fillRect(170, 7, 3, 6, GxEPD_BLACK);
}
}  // namespace ScreenHeader
