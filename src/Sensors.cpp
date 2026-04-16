#include "Sensors.h"

#include <SPI.h>
#include <Wire.h>

#include <7semi_BME690.h>
#include <Adafruit_MAX1704X.h>
#include <RV-3028-C7.h>
#include <SensirionI2cStcc4.h>
#include <math.h>
#include <stdio.h>

#include "AppConfig.h"
#include "SparkFun_BMV080_Arduino_Library.h"

namespace
{
constexpr int16_t SENSOR_NO_ERROR = 0;

SparkFunBMV080SPI bmv080;
BME690_7semi bme(0x76, BME690_7semi::MODE_I2C);
SensirionI2cStcc4 stcc4;
Adafruit_MAX17048 maxlipo;
RV3028 rtc;

bool powerRailEnabled = false;
bool wireInitialized = false;
bool wire1Initialized = false;
bool measurementDevicesReady = false;
bool batteryGaugeReady = false;
bool rtcReady = false;
RTC_DATA_ATTR float lastBatteryPercent = -1.0f;

bool hasCachedBatteryPercent()
{
   return lastBatteryPercent >= 0.0f &&
          lastBatteryPercent <= 100.0f;
}

uint16_t clampToUint16(float value)
{
   if (isnan(value) || isinf(value) || value <= 0.0f)
   {
      return 0;
   }

   if (value >= 65535.0f)
   {
      return 65535;
   }

   return static_cast<uint16_t>(value);
}

uint16_t clampToUint16(int32_t value)
{
   if (value <= 0)
   {
      return 0;
   }

   if (value >= 65535)
   {
      return 65535;
   }

   return static_cast<uint16_t>(value);
}

float sanitizeBatteryPercent(float batteryPercent)
{
   if (isnan(batteryPercent) || isinf(batteryPercent))
   {
      if (hasCachedBatteryPercent())
      {
         return lastBatteryPercent;
      }

      return 0.0f;
   }

   if (batteryPercent < 0.0f)
   {
      return 0.0f;
   }

   if (batteryPercent > 100.0f)
   {
      return 100.0f;
   }

   return batteryPercent;
}

void ensurePowerRail()
{
   if (powerRailEnabled)
   {
      return;
   }

   pinMode(AppConfig::EN_LDO2_PIN, OUTPUT);
   digitalWrite(AppConfig::EN_LDO2_PIN, HIGH);
   delay(200);
   powerRailEnabled = true;
}

void ensureWire()
{
   if (wireInitialized)
   {
      return;
   }

   Wire.begin(AppConfig::SDA_PIN0,
              AppConfig::SCL_PIN0,
              AppConfig::I2C_FREQ_0);
   wireInitialized = true;
}

void ensureWire1()
{
   if (wire1Initialized)
   {
      return;
   }

   Wire1.begin(AppConfig::SDA_PIN1,
               AppConfig::SCL_PIN1,
               AppConfig::I2C_FREQ_1);
   wire1Initialized = true;
}

void queueStcc4SingleShot()
{
   const int16_t error = stcc4.measureSingleShot();

   if (error != SENSOR_NO_ERROR)
   {
      Serial.printf("STCC4 single shot failed: %d\n", error);
   }
}

void ensureMeasurementDevices()
{
   if (measurementDevicesReady)
   {
      return;
   }

   ensurePowerRail();
   ensureWire();
   ensureWire1();

   pinMode(AppConfig::BMV_CS_PIN, OUTPUT);
   digitalWrite(AppConfig::BMV_CS_PIN, HIGH);

   pinMode(AppConfig::EPD_CS, OUTPUT);
   digitalWrite(AppConfig::EPD_CS, HIGH);

   if (!bme.begin(Wire))
   {
      Serial.println("BME690 failed");
      while (true)
      {
         delay(1000);
      }
   }

   stcc4.begin(Wire1, STCC4_I2C_ADDR_64);
   stcc4.stopContinuousMeasurement();
   queueStcc4SingleShot();

   bmv080.begin(AppConfig::BMV_CS_PIN, SPI);
   bmv080.init();
   bmv080.setMode(SF_BMV080_MODE_CONTINUOUS);
   delay(2000);

   measurementDevicesReady = true;
}

void ensureBatteryGauge()
{
   if (batteryGaugeReady)
   {
      return;
   }

   ensurePowerRail();
   ensureWire1();
   maxlipo.begin(&Wire1);
   batteryGaugeReady = true;
}

void ensureRtc()
{
   if (rtcReady)
   {
      return;
   }

   ensurePowerRail();
   ensureWire1();
   rtc.begin(Wire1);
   rtcReady = true;
}

void readBmv(SensorReadings& readings)
{
   digitalWrite(AppConfig::BMV_CS_PIN, LOW);

   if (bmv080.readSensor())
   {
      readings.pm1 = clampToUint16(bmv080.PM1());
      readings.pm25 = clampToUint16(bmv080.PM25());
      readings.pm10 = clampToUint16(bmv080.PM10());
   }

   digitalWrite(AppConfig::BMV_CS_PIN, HIGH);
}

void readBme(SensorReadings& readings)
{
   const unsigned long start = millis();

   while (!bme.readSensorData())
   {
      if (millis() - start > 1000)
      {
         Serial.println("BME timeout");
         break;
      }

      delay(20);
   }

   readings.temperature = bme.getTemperature();
   readings.humidity = bme.getHumidity();
   readings.pressure = bme.getPressure();
}

void readStcc4(SensorReadings& readings)
{
   int16_t co2ppm = 0;
   float temperature = 0.0f;
   float humidity = 0.0f;
   uint16_t status = 0;
   int16_t error =
       stcc4.readMeasurement(co2ppm, temperature, humidity, status);

   if (error != SENSOR_NO_ERROR)
   {
      delay(150);
      error = stcc4.readMeasurement(co2ppm,
                                    temperature,
                                    humidity,
                                    status);
   }

   if (error != SENSOR_NO_ERROR)
   {
      Serial.printf("STCC4 read failed: %d\n", error);
      queueStcc4SingleShot();
      return;
   }

   readings.co2ppm = clampToUint16(co2ppm);
   queueStcc4SingleShot();
}

void readBattery(SensorReadings& readings)
{
   readings.batteryPercent = Sensors::readBatteryPercent();
}

void readRtc(SensorReadings& readings)
{
   ensureRtc();

   if (!rtc.updateTime())
   {
      readings.timestamp[0] = '\0';
      return;
   }

   snprintf(readings.timestamp,
            sizeof(readings.timestamp),
            "%s",
            rtc.stringTimeStamp());
}
}  // namespace

namespace Sensors
{
void wake()
{
   ensureMeasurementDevices();
   ensureBatteryGauge();
   ensureRtc();
}

void init()
{
   wake();
}

void sleep()
{
   if (!powerRailEnabled)
   {
      return;
   }

   digitalWrite(AppConfig::EN_LDO2_PIN, LOW);
   powerRailEnabled = false;
   measurementDevicesReady = false;
   batteryGaugeReady = false;
   rtcReady = false;
}

float readBatteryPercent()
{
   ensureBatteryGauge();

   const float batteryPercent =
       sanitizeBatteryPercent(maxlipo.cellPercent());
   lastBatteryPercent = batteryPercent;
   return batteryPercent;
}

float batteryPercentForMenu()
{
   if (hasCachedBatteryPercent())
   {
      return lastBatteryPercent;
   }

   return readBatteryPercent();
}

SensorReadings readAll()
{
   wake();

   SensorReadings readings;

   readBmv(readings);
   readBme(readings);
   readStcc4(readings);
   readBattery(readings);
   readRtc(readings);

   return readings;
}
}  // namespace Sensors
