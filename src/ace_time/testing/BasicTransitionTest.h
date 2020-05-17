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
    /**
     * Check if the data from the third party library is equal to the values
     * expected from AceTime. We use custom assertion() logic instead of using
     * AUnit's built-in assertEqual() methods for 2 reasons:
     *
     *  1) We want to test all test items in validation_data.cpp for each zone,
     *  instead of terminating the test after the first failure.
     *  2) We want error messages with far more context information so that we
     *  can track down the exact test item that failed.
     */
    void assertValid(
        const basic::ZoneInfo* const zoneInfo,
        const ValidationData* const testData,
        ValidationScope dstValidationScope,
        ValidationScope abbrevValidationScope) {

      BasicZoneProcessor zoneProcessor;
      TimeZone tz = TimeZone::forZoneInfo(zoneInfo, &zoneProcessor);

      bool passed = true;
      for (uint16_t i = 0; i < testData->numItems; i++) {
        const ValidationItem& item = testData->items[i];
        acetime_t epochSeconds = item.epochSeconds;
        ZonedDateTime dt = ZonedDateTime::forEpochSeconds(epochSeconds, tz);

        checkComponent(passed, i, item, "year", dt.year(), item.year);
        checkComponent(passed, i, item, "month", dt.month(), item.month);
        checkComponent(passed, i, item, "day", dt.day(), item.day);
        checkComponent(passed, i, item, "hour", dt.hour(), item.hour);
        checkComponent(passed, i, item, "minute", dt.minute(), item.minute);
        checkComponent(passed, i, item, "second", dt.second(), item.second);

        TimeOffset timeOffset = tz.getUtcOffset(epochSeconds);
        checkComponent(passed, i, item, "UTC",
            timeOffset.toMinutes(), item.timeOffsetMinutes);

        if ((dstValidationScope == ValidationScope::kAll)
            || ((dstValidationScope == ValidationScope::kExternal)
              && (item.type == 'A' || item.type == 'B'))) {
          TimeOffset deltaOffset = tz.getDeltaOffset(epochSeconds);
          checkComponent(passed, i, item, "DST",
              deltaOffset.toMinutes(), item.deltaOffsetMinutes);
        }

        if ((abbrevValidationScope == ValidationScope::kAll)
            || ((abbrevValidationScope == ValidationScope::kExternal)
              && (item.type == 'A' || item.type == 'B'))) {
          checkAbbrev(passed, i, item, tz);
        }
      }

      if (BASIC_TRANSITION_TEST_DEBUG) {
        if (! passed) {
          zoneProcessor.log();
        }
      }
      assertTrue(passed);
    }

    void checkComponent(bool& passed, int i, const ValidationItem& item,
        const char* componentName, int aceTimeValue, int libValue) {
      if (aceTimeValue != libValue) {
        printFailedHeader(componentName, i, item);
        logging::printf("at=%d lib=%d\n", aceTimeValue, libValue);
      }
    }

    void checkAbbrev(bool& passed, int i, const ValidationItem& item,
        const TimeZone& tz) {
      if (item.abbrev == nullptr) return;

      acetime_t epochSeconds = item.epochSeconds;
      if (! aunit::internal::compareEqual(
          item.abbrev, tz.getAbbrev(epochSeconds))) {
        printFailedHeader("abbrev", i, item);
        logging::printf( "at=%s, lib=%s\n",
            tz.getAbbrev(epochSeconds),
            item.abbrev);
        passed = false;
      }
    }

    void printFailedHeader(const char* tag, uint16_t i,
        const ValidationItem& item) {
      logging::printf(
          "* Failed %s: index=%d eps=%ld "
          "%04d-%02d-%02dT%02d:%02d:%02d: ",
          tag, i, item.epochSeconds,
          item.year, item.month, item.day,
          item.hour, item.minute, item.second);
    }
};

}
}

#undef BASIC_TRANSITION_TEST_DEBUG

#endif
