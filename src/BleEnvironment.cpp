#include "BleEnvironment.h"

#include <Arduino.h>
#include <NimBLEAdvertising.h>
#include <NimBLEAdvertisementData.h>
#include <NimBLEDevice.h>
#include <math.h>

namespace
{
constexpr char DEVICE_NAME[] = "WearAware";
constexpr uint8_t CHARACTERISTIC_COUNT = 8;

NimBLEUUID serviceUUID("0000181a-0000-1000-8000-00805f9b34fc");

NimBLEUUID characteristicUUIDs[CHARACTERISTIC_COUNT] = {
    NimBLEUUID("00002A6E-0000-1000-8000-00805F9B34FB"),
    NimBLEUUID("00002A6F-0000-1000-8000-00805F9B34FB"),
    NimBLEUUID("00002A6D-0000-1000-8000-00805F9B34FB"),
    NimBLEUUID("00002B8C-0000-1000-8000-00805F9B34FB"),
    NimBLEUUID("00002BD5-0000-1000-8000-00805F9B34FB"),
    NimBLEUUID("00002BD6-0000-1000-8000-00805F9B34FB"),
    NimBLEUUID("00002BD7-0000-1000-8000-00805F9B34FB"),
    NimBLEUUID("00002A76-0000-1000-8000-00805F9B34FB"),
};

NimBLEServer* server = nullptr;
NimBLECharacteristic* characteristics[CHARACTERISTIC_COUNT] = {
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
};
bool deviceConnected = false;
bool restartAdvertisingOnDisconnect = true;
bool runtimeActive = false;

class ServerCallbacks : public NimBLEServerCallbacks
{
   void onConnect(NimBLEServer* /*server*/,
                  NimBLEConnInfo& /*connInfo*/) override
   {
      deviceConnected = true;
      Serial.println("BLE client connected");
   }

   void onDisconnect(NimBLEServer* /*server*/,
                     NimBLEConnInfo& /*connInfo*/,
                     int reason) override
   {
      deviceConnected = false;
      Serial.printf("BLE client disconnected, reason = %d\n", reason);

      if (restartAdvertisingOnDisconnect)
      {
         NimBLEDevice::startAdvertising();
         Serial.println("BLE advertising restarted");
      }
   }
};

ServerCallbacks serverCallbacks;

void clearCharacteristicPointers()
{
   for (uint8_t i = 0; i < CHARACTERISTIC_COUNT; ++i)
   {
      characteristics[i] = nullptr;
   }
}

int32_t roundedInt32(float value)
{
   if (isnan(value) || isinf(value))
   {
      return 0;
   }

   return static_cast<int32_t>(lroundf(value));
}

void setInt32Value(NimBLECharacteristic* characteristic, int32_t value)
{
   characteristic->setValue(reinterpret_cast<const uint8_t*>(&value),
                            sizeof(value));
}

void logReadings(const SensorReadings& readings)
{
   Serial.println("BLE environment update:");
   Serial.printf("Temp: %ld\n",
                 static_cast<long>(roundedInt32(readings.temperature)));
   Serial.printf("Hum : %ld\n",
                 static_cast<long>(roundedInt32(readings.humidity)));
   Serial.printf("Pres: %ld\n",
                 static_cast<long>(roundedInt32(readings.pressure)));
   Serial.printf("CO2 : %u\n", readings.co2ppm);
   Serial.printf("PM1 : %u\n", readings.pm1);
   Serial.printf("PM25: %u\n", readings.pm25);
   Serial.printf("PM10: %u\n", readings.pm10);
   Serial.printf("Batt: %ld\n",
                 static_cast<long>(roundedInt32(readings.batteryPercent)));
}

void publishReadings(const SensorReadings& readings)
{
   if (!runtimeActive)
   {
      return;
   }

   for (uint8_t i = 0; i < CHARACTERISTIC_COUNT; ++i)
   {
      if (characteristics[i] == nullptr)
      {
         return;
      }
   }

   setInt32Value(characteristics[0], roundedInt32(readings.temperature));
   setInt32Value(characteristics[1], roundedInt32(readings.humidity));
   setInt32Value(characteristics[2], roundedInt32(readings.pressure));
   setInt32Value(characteristics[3], static_cast<int32_t>(readings.co2ppm));
   setInt32Value(characteristics[4], static_cast<int32_t>(readings.pm1));
   setInt32Value(characteristics[5], static_cast<int32_t>(readings.pm25));
   setInt32Value(characteristics[6], static_cast<int32_t>(readings.pm10));
   setInt32Value(characteristics[7],
                 roundedInt32(readings.batteryPercent));

   if (!deviceConnected)
   {
      return;
   }

   for (uint8_t i = 0; i < CHARACTERISTIC_COUNT; ++i)
   {
      characteristics[i]->notify();
   }
}
}  // namespace

namespace BleEnvironment
{
bool start(const SensorReadings& initialReadings)
{
   runtimeActive = false;
   server = nullptr;
   deviceConnected = false;
   restartAdvertisingOnDisconnect = true;
   clearCharacteristicPointers();

   if (NimBLEDevice::isInitialized())
   {
      NimBLEDevice::deinit(true);
   }

   if (!NimBLEDevice::init(DEVICE_NAME))
   {
      Serial.println("Failed to initialize BLE");
      return false;
   }

   NimBLEDevice::setDeviceName(DEVICE_NAME);
   NimBLEDevice::setPower(ESP_PWR_LVL_P9);
   server = NimBLEDevice::createServer();
   server->setCallbacks(&serverCallbacks);

   NimBLEService* service = server->createService(serviceUUID);

   for (uint8_t i = 0; i < CHARACTERISTIC_COUNT; ++i)
   {
      characteristics[i] = service->createCharacteristic(
          characteristicUUIDs[i],
          NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
   }

   service->start();

   NimBLEAdvertising* advertising = NimBLEDevice::getAdvertising();
   advertising->clearData();
   advertising->enableScanResponse(true);

   NimBLEAdvertisementData advertisementData;
   advertisementData.setFlags(BLE_HS_ADV_F_DISC_GEN |
                              BLE_HS_ADV_F_BREDR_UNSUP);
   advertisementData.setName(DEVICE_NAME);

   NimBLEAdvertisementData scanResponseData;
   scanResponseData.addServiceUUID(serviceUUID);

   const bool advDataOk =
       advertising->setAdvertisementData(advertisementData);
   const bool scanDataOk =
       advertising->setScanResponseData(scanResponseData);
   const bool advertisingStarted = NimBLEDevice::startAdvertising();

   Serial.printf("BLE adv data: %s, scan data: %s, started: %s\n",
                 advDataOk ? "ok" : "failed",
                 scanDataOk ? "ok" : "failed",
                 advertisingStarted ? "yes" : "no");

   if (!advDataOk || !scanDataOk || !advertisingStarted)
   {
      stop();
      return false;
   }

   runtimeActive = true;
   publishReadings(initialReadings);
   Serial.println("BLE advertising started");
   logReadings(initialReadings);
   return true;
}

void publish(const SensorReadings& readings)
{
   publishReadings(readings);
   logReadings(readings);
}

void stop()
{
   restartAdvertisingOnDisconnect = false;
   deviceConnected = false;
   runtimeActive = false;

   if (server != nullptr)
   {
      server->stopAdvertising();
   }
   else if (NimBLEDevice::isInitialized())
   {
      NimBLEDevice::stopAdvertising();
   }

   if (NimBLEDevice::isInitialized())
   {
      NimBLEDevice::deinit(true);
   }

   server = nullptr;
   clearCharacteristicPointers();
   Serial.println("BLE stopped");
}

bool isRunning()
{
   return runtimeActive;
}
}  // namespace BleEnvironment
