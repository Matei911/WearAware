#include "DisplayScreens.h"

#include <Fonts/FreeMonoBold9pt7b.h>

#include "BleEnvironment.h"
#include "images.h"

namespace
{
constexpr int BLE_ICON_X = 10;
constexpr int BLE_ICON_Y = 1;
constexpr int BLE_ICON_WIDTH = 15;
constexpr int BLE_ICON_HEIGHT = 15;

int batteryFillWidth(float batteryPercent)
{
   const int fill =
       static_cast<int>((batteryPercent / 100.0f) * 19.0f);

   return constrain(fill, 0, 19);
}

const uint8_t* bleIconBitmap()
{
   return BleEnvironment::isRunning() ? ble_on : ble_off;
}
}  // namespace

namespace ScreenHeader
{
void draw(WearAwareDisplay& display, float batteryPercent)
{
   display.setTextColor(GxEPD_BLACK);
   display.setFont(&FreeMonoBold9pt7b);
   display.drawXBitmap(BLE_ICON_X,
                       BLE_ICON_Y,
                       bleIconBitmap(),
                       BLE_ICON_WIDTH,
                       BLE_ICON_HEIGHT,
                       GxEPD_BLACK);

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
