#include "DisplayManager.h"

#include <SPI.h>

#include "AppConfig.h"
#include "DisplayScreens.h"

namespace
{
   WearAwareDisplay display(GxEPD2_154_GDEY0154D67(AppConfig::EPD_CS,
                                                   AppConfig::EPD_DC,
                                                   AppConfig::EPD_RST,
                                                   AppConfig::EPD_BUSY));

   RTC_DATA_ATTR uint8_t refreshCycle = 0;
   bool isInitialized = false;

   bool shouldUseFullRefresh()
   {
      return refreshCycle == 0;
   }

   void advanceRefreshCycle()
   {
      refreshCycle = (refreshCycle + 1) % AppConfig::REFRESH_CYCLE_LENGTH;
   }

   void selectRefreshWindow()
   {
      if (shouldUseFullRefresh())
      {
         display.setFullWindow();
         return;
      }

      display.setPartialWindow(0,
                               0,
                               AppConfig::DISPLAY_WIDTH,
                               AppConfig::DISPLAY_HEIGHT);
   }
} // namespace

namespace DisplayManager
{
   void init()
   {
      if (isInitialized)
      {
         return;
      }

      pinMode(AppConfig::DISPLAY_POWER_PIN, OUTPUT);
      digitalWrite(AppConfig::DISPLAY_POWER_PIN, LOW);
      delay(500);

      pinMode(AppConfig::BMV_CS_PIN, OUTPUT);
      digitalWrite(AppConfig::BMV_CS_PIN, HIGH);
      pinMode(AppConfig::EPD_CS, OUTPUT);
      digitalWrite(AppConfig::EPD_CS, HIGH);

      SPI.begin(AppConfig::EPD_SCK,
                AppConfig::EPD_MISO,
                AppConfig::EPD_MOSI);

      display.epd2.selectSPI(
          SPI, SPISettings(4000000, MSBFIRST, SPI_MODE0));

      display.init(115200, false, 2, false);
      display.setRotation(AppConfig::DISPLAY_ROTATION);
      isInitialized = true;
   }

   void forceNextFullRefresh()
   {
      refreshCycle = 0;
   }

   void renderHomeScreen(const SensorReadings &readings)
   {
      selectRefreshWindow();

      display.firstPage();
      do
      {
         HomeScreen::draw(display, readings);
      } while (display.nextPage());

      advanceRefreshCycle();
   }

   void renderSamplingMenu(SamplingInterval selectedInterval,
                           bool usePartialRefresh,
                           const SensorReadings &readings)
   {
      if (usePartialRefresh)
      {
         display.setPartialWindow(0,
                                  0,
                                  AppConfig::DISPLAY_WIDTH,
                                  AppConfig::DISPLAY_HEIGHT);
      }
      else
      {
         display.setFullWindow();
      }

      display.firstPage();
      do
      {
         SamplingMenuScreen::draw(display, selectedInterval, readings);
      } while (display.nextPage());
   }
} // namespace DisplayManager
