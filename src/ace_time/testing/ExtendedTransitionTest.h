/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#ifndef ACE_TIME_EXTENDED_TRANSITION_TEST_H
#define ACE_TIME_EXTENDED_TRANSITION_TEST_H

#include <AUnit.h>
#include <AceTime.h>
#include "ValidationDataType.h"
#include "../common/logging.h"

#define EXTENDED_TRANSITION_TEST_DEBUG 0

namespace ace_time {
namespace testing {

class ExtendedTransitionTest: public aunit::TestOnce {
  protected:
    void assertValid(
        const extended::ZoneInfo* const zoneInfo,
        const ValidationData* testData,
        bool validateDst = false) {

      if (EXTENDED_TRANSITION_TEST_DEBUG) {
        enableVerbosity(aunit::Verbosity::kAssertionPassed);
      }

      ExtendedZoneProcessor zoneProcessor;
      zoneProcessor.resetTransitionHighWater();
      TimeZone tz = TimeZone::forZoneInfo(zoneInfo, &zoneProcessor);

      // Assert that each epoch_second produces the expected yMdhms
      // components when converted through ZonedDataTime class.
      for (uint16_t i = 0; i < testData->numItems; i++) {
        const ValidationItem& item = testData->items[i];
        acetime_t epochSeconds = item.epochSeconds;
        if (EXTENDED_TRANSITION_TEST_DEBUG) {
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
        if (EXTENDED_TRANSITION_TEST_DEBUG) {
          zoneProcessor.log();
        }

        // Verify timeOffset
        assertEqual(item.timeOffsetMinutes, timeOffset.toMinutes());

        // Verify DST offset.
        if (validateDst) {
          TimeOffset deltaOffset = tz.getDeltaOffset(epochSeconds);
          assertEqual(item.deltaOffsetMinutes, deltaOffset.toMinutes());
        }

        // Verify date components
        ZonedDateTime dt = ZonedDateTime::forEpochSeconds(epochSeconds, tz);
        assertEqual(item.year, dt.year());
        assertEqual(item.month, dt.month());
        assertEqual(item.day, dt.day());
        assertEqual(item.hour, dt.hour());
        assertEqual(item.minute, dt.minute());
        assertEqual(item.second, dt.second());

        // Verify abbreviation if it is defined.
        if (item.abbrev != nullptr) {
          assertEqual(item.abbrev, tz.getAbbrev(epochSeconds));
        }
      }

      // Assert that size of the internal Transitions buffer never got
      // above the expected buffer size. The buffer size is only relevant for
      // the ExtendedZoneProcessor class.
      //
      // TODO: In theory, this should be BasicZone(zoneInfo).transitionBufSize()
      // but this code works only on Linux or MacOS, so doesn't really matter.
      assertLess(zoneProcessor.getTransitionHighWater(),
        zoneInfo->transitionBufSize);
    }
};

}
}

#undef EXTENDED_TRANSITION_TEST_DEBUG

#endif
