/*
 * MIT License
 * Copyright (c) 2019 Brian T. Park
 */

#ifndef ACE_TIME_BASIC_TRANSITION_TEST_H
#define ACE_TIME_BASIC_TRANSITION_TEST_H

#include <AUnitVerbose.h>
#include <AceTime.h>
#include "ValidationDataType.h"
#include "ValidationScope.h"

#define BASIC_TRANSITION_TEST_DEBUG 0

namespace ace_time {
namespace testing {

class BasicTransitionTest: public aunit::TestOnce {
  protected:
    void assertValid(
        const basic::ZoneInfo* const zoneInfo,
        const ValidationData* const testData,
        ValidationScope dstValidationScope,
        ValidationScope abbrevValidationScope) {

      if (BASIC_TRANSITION_TEST_DEBUG) {
        enableVerbosity(aunit::Verbosity::kAssertionPassed);
      }

      BasicZoneProcessor zoneProcessor;
      TimeZone tz = TimeZone::forZoneInfo(zoneInfo, &zoneProcessor);
      for (uint16_t i = 0; i < testData->numItems; i++) {
        const ValidationItem& item = testData->items[i];
        acetime_t epochSeconds = item.epochSeconds;

        // Call getUtcOffset() to prime the ZoneProcessor cache, before
        // zoneProcessor.log() is called.
        TimeOffset timeOffset = tz.getUtcOffset(epochSeconds);

        if (BASIC_TRANSITION_TEST_DEBUG) {
          printTestInfo(i, epochSeconds);
          zoneProcessor.log();
        }

        // Verify total UTC timeOffset
        if (item.timeOffsetMinutes != timeOffset.toMinutes()) {
          printTestInfo(i, epochSeconds);
        }
        assertEqual(item.timeOffsetMinutes, timeOffset.toMinutes());

        // Verify date components
        ZonedDateTime dt = ZonedDateTime::forEpochSeconds(epochSeconds, tz);
        assertEqual(item.year, dt.year());
        assertEqual(item.month, dt.month());
        assertEqual(item.day, dt.day());
        assertEqual(item.hour, dt.hour());
        assertEqual(item.minute, dt.minute());
        assertEqual(item.second, dt.second());

        // Verify DST offset if enabled.
        if ((dstValidationScope == ValidationScope::kAll)
            || ((dstValidationScope == ValidationScope::kExternal)
              && (item.type == 'A' || item.type == 'B'))) {
          TimeOffset deltaOffset = tz.getDeltaOffset(epochSeconds);
          if (item.deltaOffsetMinutes != deltaOffset.toMinutes()) {
            printTestInfo(i, epochSeconds);
          }
          assertEqual(item.deltaOffsetMinutes, deltaOffset.toMinutes());
        }

        // Verify abbreviation if enabled.
        if ((abbrevValidationScope == ValidationScope::kAll)
            || ((abbrevValidationScope == ValidationScope::kExternal)
              && (item.type == 'A' || item.type == 'B'))) {
          if (item.abbrev != nullptr) {
            if (! aunit::internal::compareEqual(
                item.abbrev, tz.getAbbrev(epochSeconds))) {
              printTestInfo(i, epochSeconds);
            }
            assertEqual(item.abbrev, tz.getAbbrev(epochSeconds));
          }
        }
      }
    }

    void printTestInfo(uint16_t i, acetime_t epochSeconds) {
      ace_time::logging::printf("==== failed at index: %d; ", i);
      if (sizeof(acetime_t) == sizeof(int)) {
        ace_time::logging::printf("epochSeconds: %d\r\n", epochSeconds);
      } else {
        ace_time::logging::printf("epochSeconds: %ld\r\n", epochSeconds);
      }
    }
};

}
}

#undef BASIC_TRANSITION_TEST_DEBUG

#endif
