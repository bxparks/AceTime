#ifndef VALIDATION_TEST_EXTENDED_TRANSITION_TEST_H
#define VALIDATION_TEST_EXTENDED_TRANSITION_TEST_H

#include <AUnit.h>
#include "ValidationDataType.h"
#include "ace_time/common/logging.h"

#define DEBUG 0

class TransitionTest: public aunit::TestOnce {
  protected:
    void assertValid(const ValidationData* testData) {
      if (DEBUG) {
        enableVerbosity(aunit::Verbosity::kAssertionPassed);
      }
      assertTrue(true);

      const extended::ZoneInfo* zoneInfo = testData->zoneInfo;
      ExtendedZoneProcessor zoneProcessor;
      zoneProcessor.resetTransitionHighWater();
      TimeZone tz = TimeZone::forZoneInfo(zoneInfo, &zoneProcessor);

      // Assert that each epoch_second produces the expected yMdhms
      // components when converted through ZonedDataTime class.
      for (uint16_t i = 0; i < testData->numItems; i++) {
        const ValidationItem& item = testData->items[i];
        acetime_t epochSeconds = item.epochSeconds;
        if (DEBUG) {
          ace_time::logging::printf("==== test index: %d\n", i);
          if (sizeof(acetime_t) == sizeof(int)) {
            ace_time::logging::printf("epochSeconds: %d", epochSeconds);
          } else {
            ace_time::logging::printf("epochSeconds: %ld", epochSeconds);
          }
          ace_time::logging::printf("; %d-%d-%dT%d:%d:%d\n",
            item.year,
            item.month,
            item.day,
            item.hour,
            item.minute,
            item.second);
        }

        TimeOffset timeOffset = tz.getUtcOffset(epochSeconds);
        if (DEBUG) zoneProcessor.log();

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

      // Assert that size of the internal Transitions buffer never got
      // above the expected buffer size.
      // TODO: In theory, this should be BasicZone(zoneInfo).transitionBufSize()
      // but this code works only on Linux or MacOS, so doesn't really matter.
      assertLess(zoneProcessor.getTransitionHighWater(),
        zoneInfo->transitionBufSize);
    }
};

#undef DEBUG

#endif
