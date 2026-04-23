#pragma once

#include <Arduino.h>

#ifndef WEARAWARE_ENABLE_BLE_DEBUG_LOGS
#define WEARAWARE_ENABLE_BLE_DEBUG_LOGS 0
#endif

#ifndef WEARAWARE_ENABLE_SENSOR_DEBUG_LOGS
#define WEARAWARE_ENABLE_SENSOR_DEBUG_LOGS 1
#endif

namespace AppConfig
{
constexpr int EN_LDO2_PIN = 1;
constexpr int DISPLAY_POWER_PIN = 37;

constexpr int BMV_CS_PIN = 36;

constexpr int SDA_PIN0 = 17;
constexpr int SCL_PIN0 = 14;
constexpr uint32_t I2C_FREQ_0 = 100000;

constexpr int SDA_PIN1 = 8;
constexpr int SCL_PIN1 = 9;
constexpr uint32_t I2C_FREQ_1 = 100000;

constexpr int EPD_SCK = 12;
constexpr int EPD_MISO = 13;
constexpr int EPD_MOSI = 11;
constexpr int EPD_CS = 5;
constexpr int EPD_DC = 4;
constexpr int EPD_RST = 3;
constexpr int EPD_BUSY = 2;

constexpr int BUTTON_PIN_1 = 0;
constexpr int BUTTON_PIN_2 = 34;
constexpr int BUTTON_PIN_3 = 33;

constexpr uint16_t DISPLAY_WIDTH = 200;
constexpr uint16_t DISPLAY_HEIGHT = 200;
constexpr uint8_t DISPLAY_ROTATION = 0;
constexpr uint8_t REFRESH_CYCLE_LENGTH = 2;

constexpr int8_t BLE_TX_POWER_DBM = 0;
constexpr uint16_t BLE_ADV_INTERVAL_MIN_UNITS = 800;
constexpr uint16_t BLE_ADV_INTERVAL_MAX_UNITS = 1600;
constexpr uint16_t BLE_CONN_INTERVAL_MIN_UNITS = 48;
constexpr uint16_t BLE_CONN_INTERVAL_MAX_UNITS = 96;
constexpr uint16_t BLE_CONN_LATENCY = 4;
constexpr uint16_t BLE_CONN_TIMEOUT_UNITS = 600;

constexpr uint16_t APP_MODE_ADVERTISING_IDLE_DELAY_MS = 250;
constexpr uint16_t APP_MODE_CONNECTED_IDLE_DELAY_MS = 100;
constexpr uint16_t APP_MODE_SETTLE_DELAY_MS = 20;
}  // namespace AppConfig

static_assert(AppConfig::REFRESH_CYCLE_LENGTH > 0,
              "REFRESH_CYCLE_LENGTH must be at least 1");
