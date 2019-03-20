#ifndef VALIDATION_TEST_EXTENDED_TRANSITION_TEST_H
#define VALIDATION_TEST_EXTENDED_TRANSITION_TEST_H

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

      const zonedbx::ZoneInfo* zoneInfo = testData->zoneInfo;
      ExtendedZoneSpecifier zoneSpecifier(zoneInfo);
      TimeZone tz(&zoneSpecifier);
      for (uint16_t i = 0; i < testData->numItems; i++) {
        const ValidationItem& item = testData->items[i];
        acetime_t epochSeconds = item.epochSeconds;
        if (DEBUG) {
          ace_time::logging::println("==== test index: %d", i);
          if (sizeof(acetime_t) == sizeof(int)) {
            ace_time::logging::print("epochSeconds: %d", epochSeconds);
          } else {
            ace_time::logging::print("epochSeconds: %ld", epochSeconds);
          }
          ace_time::logging::println("; %d-%d-%dT%d:%d:%d",
            item.yearTiny + LocalDate::kEpochYear,
            item.month,
            item.day,
            item.hour,
            item.minute,
            item.second);
        }

        UtcOffset utcOffset = zoneSpecifier.getUtcOffset(epochSeconds);
        if (DEBUG) zoneSpecifier.log();

        // Verify utcOffset
        assertEqual(item.utcOffsetMinutes, utcOffset.toMinutes());

        // Verify date components
        ZonedDateTime dt = ZonedDateTime::forEpochSeconds(epochSeconds, tz);
        assertEqual(item.yearTiny, dt.yearTiny());
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
