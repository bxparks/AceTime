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

      const common::ZoneInfo* zoneInfo = testData->zoneInfo;
      AutoZoneSpecifier zoneSpecifier(zoneInfo);
      for (uint16_t i = 0; i < testData->numItems; i++) {
        const ValidationItem& item = testData->items[i];
        acetime_t epochSeconds = item.epochSeconds;
        UtcOffset utcOffset = zoneSpecifier.getUtcOffset(epochSeconds);
        if (DEBUG) {
          ace_time::common::logger("epochSeconds: %ld", epochSeconds);
          zoneSpecifier.log();
        }
        assertEqual(item.utcOffsetMinutes, utcOffset.toMinutes());
      }
    }
};

#endif
