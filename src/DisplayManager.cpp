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
bool spiConfigured = false;
bool displayPowered = false;

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

void ensureSpiConfigured()
{
   if (spiConfigured)
   {
      return;
   }

   pinMode(AppConfig::BMV_CS_PIN, OUTPUT);
   digitalWrite(AppConfig::BMV_CS_PIN, HIGH);
   pinMode(AppConfig::EPD_CS, OUTPUT);
   digitalWrite(AppConfig::EPD_CS, HIGH);

   SPI.begin(AppConfig::EPD_SCK,
             AppConfig::EPD_MISO,
             AppConfig::EPD_MOSI);

   display.epd2.selectSPI(
       SPI, SPISettings(4000000, MSBFIRST, SPI_MODE0));
   spiConfigured = true;
}
}  // namespace

namespace DisplayManager
{
void wake()
{
   if (displayPowered)
   {
      return;
   }

   pinMode(AppConfig::DISPLAY_POWER_PIN, OUTPUT);
   digitalWrite(AppConfig::DISPLAY_POWER_PIN, LOW);
   delay(500);

   ensureSpiConfigured();

   display.init(115200, false, 2, false);
   display.setRotation(AppConfig::DISPLAY_ROTATION);
   displayPowered = true;
   forceNextFullRefresh();
}

void init()
{
   wake();
}

void sleep()
{
   if (!displayPowered)
   {
      return;
   }

   display.hibernate();
   digitalWrite(AppConfig::DISPLAY_POWER_PIN, HIGH);
   displayPowered = false;
   forceNextFullRefresh();
}

void forceNextFullRefresh()
{
   refreshCycle = 0;
}

void renderHomeScreen(const SensorReadings& readings)
{
   wake();
   selectRefreshWindow();

   display.firstPage();
   do
   {
      HomeScreen::draw(display, readings);
   } while (display.nextPage());

   advanceRefreshCycle();
}

void renderModeMenu(SamplingMenu::ModeRow selectedRow,
                    bool usePartialRefresh,
                    const SensorReadings& readings)
{
   wake();

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
      SamplingMenuScreen::drawModeMenu(display, selectedRow, readings);
   } while (display.nextPage());
}

void renderDeviceIntervalMenu(
    SamplingMenu::DeviceIntervalRow selectedRow,
    bool usePartialRefresh,
    const SensorReadings& readings)
{
   wake();

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
      SamplingMenuScreen::drawDeviceIntervalMenu(display,
                                                 selectedRow,
                                                 readings);
   } while (display.nextPage());
}

void renderAppIntervalMenu(SamplingMenu::AppIntervalRow selectedRow,
                           bool usePartialRefresh,
                           const SensorReadings& readings)
{
   wake();

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
      SamplingMenuScreen::drawAppIntervalMenu(display,
                                              selectedRow,
                                              readings);
   } while (display.nextPage());
}

void renderWaitingForDataScreen(const SensorReadings& readings)
{
   wake();
   display.setFullWindow();

   display.firstPage();
   do
   {
      WaitingForDataScreen::draw(display, readings);
   } while (display.nextPage());
}

void renderPhonePromptScreen(const SensorReadings& readings)
{
   wake();
   display.setFullWindow();

   display.firstPage();
   do
   {
      PhonePromptScreen::draw(display, readings);
   } while (display.nextPage());
}

void renderConnectedPromptScreen(const SensorReadings& readings)
{
   wake();
   display.setFullWindow();

   display.firstPage();
   do
   {
      ConnectedPromptScreen::draw(display, readings);
   } while (display.nextPage());
}
}  // namespace DisplayManager
