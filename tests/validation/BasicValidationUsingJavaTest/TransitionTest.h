#ifndef VALIDATION_TEST_TRANSITION_TEST_H
#define VALIDATION_TEST_TRANSITION_TEST_H

#include <AUnit.h>
#include "ValidationDataType.h"
#include "ace_time/common/logger.h"

#define DEBUG 0

class TransitionTest: public aunit::TestOnce {
  protected:
    void assertValid(const ValidationData* testData) {
      if (DEBUG) {
        enableVerbosity(aunit::Verbosity::kAssertionPassed);
      }
      assertTrue(true);

      const basic::ZoneInfo* zoneInfo = testData->zoneInfo;
      BasicZoneProcessor zoneProcessor;
      TimeZone tz = TimeZone::forZoneInfo(zoneInfo, &zoneProcessor);
      for (uint16_t i = 0; i < testData->numItems; i++) {
        const ValidationItem& item = testData->items[i];
        acetime_t epochSeconds = item.epochSeconds;

        TimeOffset timeOffset = zoneProcessor.getUtcOffset(epochSeconds);
        if (DEBUG) {
          ace_time::logging::println("==== test index: %d", i);
          if (sizeof(acetime_t) == sizeof(int)) {
            ace_time::logging::println("epochSeconds: %d", epochSeconds);
          } else {
            ace_time::logging::println("epochSeconds: %ld", epochSeconds);
          }
          zoneProcessor.log();
        }

        // Verify timeOffset
        assertEqual(item.timeOffsetMinutes, timeOffset.toMinutes());

        // Verify date components
        ZonedDateTime dt = ZonedDateTime::forEpochSeconds(epochSeconds, tz);
        assertEqual(item.year, dt.year());
        assertEqual(item.month, dt.month());
        assertEqual(item.day, dt.day());
        assertEqual(item.hour, dt.hour());
        assertEqual(item.minute, dt.minute());
        assertEqual(item.second, dt.second());
      }
    }
};

#undef DEBUG

#endif
