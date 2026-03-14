#pragma once

#include <Arduino.h>

namespace PowerManager
{
void init();
bool shouldEnterSamplingMenu();
void waitForButtonsReleased();
void enterDeepSleep(uint64_t sleepDurationUs);
}  // namespace PowerManager
