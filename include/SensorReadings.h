#pragma once

#include <Arduino.h>

struct SensorReadings
{
   uint16_t pm1 = 0;
   uint16_t pm25 = 0;
   uint16_t pm10 = 0;
   uint16_t co2ppm = 0;
   float temperature = 0.0f;
   float humidity = 0.0f;
   float pressure = 0.0f;
   float batteryPercent = 0.0f;
   char timestamp[25] = "";
};
