#include "Sensors.h"

#include <SPI.h>
#include <Wire.h>

#include <7semi_BME690.h>
#include <Adafruit_MAX1704X.h>
#include <RV-3028-C7.h>
#include <SensirionI2cStcc4.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>

#include "AppConfig.h"
#include "SparkFun_BMV080_Arduino_Library.h"

namespace
{
constexpr int16_t SENSOR_NO_ERROR = 0;
constexpr uint32_t BME_REFRESH_INTERVAL_SECONDS = 5UL * 60UL;

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
RTC_DATA_ATTR bool hasCachedBmeReadings = false;
RTC_DATA_ATTR float lastBmeTemperature = 0.0f;
RTC_DATA_ATTR float lastBmeHumidity = 0.0f;
RTC_DATA_ATTR float lastBmePressure = 0.0f;
RTC_DATA_ATTR uint32_t lastBmeUnixTime = 0;
unsigned long lastBmeReadMs = 0;

#if WEARAWARE_ENABLE_SENSOR_DEBUG_LOGS
void debugLog(const char* message)
{
   Serial.printf("[sensor %lu ms] %s\n",
                 static_cast<unsigned long>(millis()),
                 message);
}

void debugLogf(const char* format, ...)
{
   char message[220];
   va_list args;
   va_start(args, format);
   vsnprintf(message, sizeof(message), format, args);
   va_end(args);

   debugLog(message);
}
#else
void debugLog(const char* /*message*/) {}
void debugLogf(const char* /*format*/, ...) {}
#endif

void debugLogDuration(const char* label, unsigned long startedAt)
{
   debugLogf("%s done in %lu ms",
             label,
             static_cast<unsigned long>(millis() - startedAt));
}

void debugLogReadings(const SensorReadings& readings)
{
   debugLogf("readAll values: PM1=%u PM2.5=%u PM10=%u CO2=%u "
             "tmp=%.2f hum=%.2f pressure=%.2f battery=%.2f "
             "timestamp=%s",
             static_cast<unsigned int>(readings.pm1),
             static_cast<unsigned int>(readings.pm25),
             static_cast<unsigned int>(readings.pm10),
             static_cast<unsigned int>(readings.co2ppm),
             readings.temperature,
             readings.humidity,
             readings.pressure,
             readings.batteryPercent,
             readings.timestamp[0] == '\0' ? "(empty)"
                                            : readings.timestamp);
}

bool hasCachedBatteryPercent()
{
   return lastBatteryPercent >= 0.0f &&
          lastBatteryPercent <= 100.0f;
}

bool hasValidBmeCache()
{
   return hasCachedBmeReadings &&
          !isnan(lastBmeTemperature) &&
          !isinf(lastBmeTemperature) &&
          !isnan(lastBmeHumidity) &&
          !isinf(lastBmeHumidity) &&
          !isnan(lastBmePressure) &&
          !isinf(lastBmePressure);
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

bool tryGetCurrentUnixTime(uint32_t& unixTime)
{
   ensureRtc();

   if (!rtc.updateTime())
   {
      return false;
   }

   unixTime = rtc.getUNIX();
   return unixTime != 0;
}

void applyCachedBmeReadings(SensorReadings& readings)
{
   readings.temperature = lastBmeTemperature;
   readings.humidity = lastBmeHumidity;
   readings.pressure = lastBmePressure;
}

void cacheBmeReadings(const SensorReadings& readings, uint32_t unixTime)
{
   lastBmeTemperature = readings.temperature;
   lastBmeHumidity = readings.humidity;
   lastBmePressure = readings.pressure;
   lastBmeUnixTime = unixTime;
   lastBmeReadMs = millis();
   hasCachedBmeReadings = true;
}

bool shouldRefreshBme(bool hasCurrentUnixTime, uint32_t currentUnixTime)
{
   if (!hasValidBmeCache())
   {
      return true;
   }

   if (hasCurrentUnixTime)
   {
      if (lastBmeUnixTime == 0 || currentUnixTime < lastBmeUnixTime)
      {
         return true;
      }

      return (currentUnixTime - lastBmeUnixTime) >=
             BME_REFRESH_INTERVAL_SECONDS;
   }

   return lastBmeReadMs == 0 ||
          (millis() - lastBmeReadMs) >=
              (BME_REFRESH_INTERVAL_SECONDS * 1000UL);
}

void readBmv(SensorReadings& readings)
{
   const unsigned long startedAt = millis();
   debugLog("BMV read start");

   digitalWrite(AppConfig::BMV_CS_PIN, LOW);

   if (bmv080.readSensor())
   {
      readings.pm1 = clampToUint16(bmv080.PM1());
      readings.pm25 = clampToUint16(bmv080.PM25());
      readings.pm10 = clampToUint16(bmv080.PM10());
      debugLogf("BMV values: PM1=%u PM2.5=%u PM10=%u",
                static_cast<unsigned int>(readings.pm1),
                static_cast<unsigned int>(readings.pm25),
                static_cast<unsigned int>(readings.pm10));
   }
   else
   {
      debugLog("BMV no new data");
   }

   digitalWrite(AppConfig::BMV_CS_PIN, HIGH);
   debugLogDuration("BMV read", startedAt);
}

void readBme(SensorReadings& readings)
{
   const unsigned long startedAt = millis();
   debugLog("BME read start");

   uint32_t currentUnixTime = 0;
   const bool hasCurrentUnixTime =
       tryGetCurrentUnixTime(currentUnixTime);
   const bool refreshBme =
       shouldRefreshBme(hasCurrentUnixTime, currentUnixTime);

   debugLogf("BME clock: rtc=%s unix=%lu refresh=%s",
             hasCurrentUnixTime ? "ok" : "missing",
             static_cast<unsigned long>(currentUnixTime),
             refreshBme ? "yes" : "no");

   if (!refreshBme)
   {
      applyCachedBmeReadings(readings);
      debugLogf("BME using cache: tmp=%.2f hum=%.2f pressure=%.2f",
                readings.temperature,
                readings.humidity,
                readings.pressure);
      debugLogDuration("BME read", startedAt);
      return;
   }

   const unsigned long start = millis();
   bool timedOut = false;

   while (!bme.readSensorData())
   {
      if (millis() - start > 1000)
      {
         Serial.println("BME timeout");
         debugLog("BME timeout while waiting for data");
         timedOut = true;
         break;
      }

      delay(20);
   }

   readings.temperature = bme.getTemperature();
   readings.humidity = bme.getHumidity();
   readings.pressure = bme.getPressure();
   cacheBmeReadings(readings,
                    hasCurrentUnixTime ? currentUnixTime : 0);
   debugLogf("BME fresh values%s: tmp=%.2f hum=%.2f pressure=%.2f",
             timedOut ? " after timeout" : "",
             readings.temperature,
             readings.humidity,
             readings.pressure);
   debugLogDuration("BME read", startedAt);
}

void readStcc4(SensorReadings& readings)
{
   const unsigned long startedAt = millis();
   debugLog("STCC4 read start");

   int16_t co2ppm = 0;
   float temperature = 0.0f;
   float humidity = 0.0f;
   uint16_t status = 0;
   int16_t error =
       stcc4.readMeasurement(co2ppm, temperature, humidity, status);

   if (error != SENSOR_NO_ERROR)
   {
      debugLogf("STCC4 first read failed: %d; retrying",
                static_cast<int>(error));
      delay(150);
      error = stcc4.readMeasurement(co2ppm,
                                    temperature,
                                    humidity,
                                    status);
   }

   if (error != SENSOR_NO_ERROR)
   {
      Serial.printf("STCC4 read failed: %d\n", error);
      debugLogf("STCC4 retry failed: %d", static_cast<int>(error));
      queueStcc4SingleShot();
      debugLogDuration("STCC4 read", startedAt);
      return;
   }

   readings.co2ppm = clampToUint16(co2ppm);
   debugLogf("STCC4 values: CO2=%u status=%u",
             static_cast<unsigned int>(readings.co2ppm),
             static_cast<unsigned int>(status));
   queueStcc4SingleShot();
   debugLogDuration("STCC4 read", startedAt);
}

void readBattery(SensorReadings& readings)
{
   const unsigned long startedAt = millis();
   debugLog("battery read start");

   readings.batteryPercent = Sensors::readBatteryPercent();
   debugLogf("battery value: %.2f%%", readings.batteryPercent);
   debugLogDuration("battery read", startedAt);
}

void readRtc(SensorReadings& readings)
{
   const unsigned long startedAt = millis();
   debugLog("RTC read start");

   ensureRtc();

   if (!rtc.updateTime())
   {
      readings.timestamp[0] = '\0';
      debugLog("RTC update failed");
      debugLogDuration("RTC read", startedAt);
      return;
   }

   snprintf(readings.timestamp,
            sizeof(readings.timestamp),
            "%s",
            rtc.stringTimeStamp());
   debugLogf("RTC timestamp: %s", readings.timestamp);
   debugLogDuration("RTC read", startedAt);
}
}  // namespace

namespace Sensors
{
void wake()
{
   const unsigned long startedAt = millis();
   debugLog("wake start");

   ensureMeasurementDevices();
   ensureBatteryGauge();
   ensureRtc();
   debugLogDuration("wake", startedAt);
}

void init()
{
   wake();
}

void sleep()
{
   if (measurementDevicesReady)
   {
      pinMode(AppConfig::BMV_CS_PIN, OUTPUT);
      digitalWrite(AppConfig::BMV_CS_PIN, HIGH);
   }

   if (wireInitialized)
   {
      Wire.end();
      wireInitialized = false;
   }

   if (wire1Initialized)
   {
      Wire1.end();
      wire1Initialized = false;
   }

   pinMode(AppConfig::SDA_PIN0, INPUT);
   pinMode(AppConfig::SCL_PIN0, INPUT);
   pinMode(AppConfig::SDA_PIN1, INPUT);
   pinMode(AppConfig::SCL_PIN1, INPUT);
   pinMode(AppConfig::BMV_CS_PIN, INPUT);

   if (powerRailEnabled)
   {
      digitalWrite(AppConfig::EN_LDO2_PIN, LOW);
      powerRailEnabled = false;
   }

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
   const unsigned long startedAt = millis();
   debugLog("readAll start");

   wake();

   SensorReadings readings;

   readBmv(readings);
   readBme(readings);
   readStcc4(readings);
   readBattery(readings);
   readRtc(readings);

   debugLogReadings(readings);
   debugLogDuration("readAll", startedAt);
   return readings;
}
}  // namespace Sensors
