#include "SamplingSettings.h"

namespace
{
constexpr uint64_t SLEEP_DURATIONS_US[] = {
    300ULL * 1000000ULL,
    60ULL * 1000000ULL,
    30ULL * 1000000ULL,
};

constexpr uint8_t INTERVAL_COUNT =
    sizeof(SLEEP_DURATIONS_US) / sizeof(SLEEP_DURATIONS_US[0]);

RTC_DATA_ATTR uint8_t selectedIntervalIndex =
    static_cast<uint8_t>(SamplingInterval::FiveMinutes);

uint8_t sanitizeIndex(uint8_t index)
{
   return index < INTERVAL_COUNT ? index : 0;
}

uint8_t indexFromInterval(SamplingInterval interval)
{
   return sanitizeIndex(static_cast<uint8_t>(interval));
}

SamplingInterval intervalFromIndex(uint8_t index)
{
   return static_cast<SamplingInterval>(sanitizeIndex(index));
}
}  // namespace

namespace SamplingSettings
{
void init()
{
   selectedIntervalIndex = sanitizeIndex(selectedIntervalIndex);
}

SamplingInterval getSelectedInterval()
{
   return intervalFromIndex(selectedIntervalIndex);
}

void setSelectedInterval(SamplingInterval interval)
{
   selectedIntervalIndex = indexFromInterval(interval);
}

SamplingInterval nextInterval(SamplingInterval interval)
{
   const uint8_t nextIndex =
       (indexFromInterval(interval) + 1) % INTERVAL_COUNT;

   return intervalFromIndex(nextIndex);
}

SamplingInterval previousInterval(SamplingInterval interval)
{
   const uint8_t currentIndex = indexFromInterval(interval);
   const uint8_t previousIndex =
       (currentIndex + INTERVAL_COUNT - 1) % INTERVAL_COUNT;

   return intervalFromIndex(previousIndex);
}

uint64_t selectedSleepDurationUs()
{
   return SLEEP_DURATIONS_US[selectedIntervalIndex];
}
}  // namespace SamplingSettings
