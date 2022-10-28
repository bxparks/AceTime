#line 2 "ExtendedZoneProcessorTransitionTest.ino"

#include <Arduino.h>
#include <AUnit.h>
#include <AceTime.h>
#include "EuropeLisbon.h"
#include <ace_time/testing/EpochYearContext.h>

using namespace ace_time;

//----------------------------------------------------------------------------

ExtendedZoneProcessor zoneProcessor;

extended::ZoneRegistrar zoneRegistrar(
    zonedbx::kZoneRegistrySize,
    zonedbx::kZoneRegistry);

//----------------------------------------------------------------------------

/**
 * Check that all Transitions for all years, for all zones in the zonedbx
 * database:
 *
 *  1) are sorted with respect to startEpochSeconds
 *  2) are unique with respect to startEpochSeconds
 */
class TransitionValidation : public aunit::TestOnce {
  public:
    void validateZone(int16_t startYear, int16_t untilYear) {
      for (int16_t year = startYear; year < untilYear; year++) {

        bool status = zoneProcessor.initForYear(year);
        if (! status) {
          assertNoFatalFailure(failWithMessage(year, "initForYear() failed"));
        }

        ExtendedZoneProcessor::Transition** start =
            zoneProcessor.mTransitionStorage.getActivePoolBegin();
        ExtendedZoneProcessor::Transition** end =
            zoneProcessor.mTransitionStorage.getActivePoolEnd();

        assertNoFatalFailure(checkSortedTransitions(year, start, end));
        assertNoFatalFailure(checkUniqueTransitions(year, start, end));
      }
    }

  private:
    void checkSortedTransitions(
        int16_t year,
        ExtendedZoneProcessor::Transition** start,
        ExtendedZoneProcessor::Transition** end) {

      ExtendedZoneProcessor::Transition* prev = nullptr;
      for (ExtendedZoneProcessor::Transition** iter = start;
          iter != end;
          iter++) {
        ExtendedZoneProcessor::Transition* t = *iter;
        if (prev) {
          if (prev->startEpochSeconds > t->startEpochSeconds) {
            assertNoFatalFailure(failWithMessage(
                year, "Transition is not sorted"));
          }
        }
        prev = t;
      }
    }

    void checkUniqueTransitions(
        int16_t year,
        ExtendedZoneProcessor::Transition** start,
        ExtendedZoneProcessor::Transition** end) {

      ExtendedZoneProcessor::Transition* prev = nullptr;
      for (ExtendedZoneProcessor::Transition** iter = start;
          iter != end;
          iter++) {
        ExtendedZoneProcessor::Transition* t = *iter;
        if (prev) {
          if (prev->startEpochSeconds == t->startEpochSeconds) {
            assertNoFatalFailure(failWithMessage(
                year, "Transition is duplicated"));
          }
        }
        prev = t;
      }
    }

    void failWithMessage(int16_t year, const char* msg) {
      SERIAL_PORT_MONITOR.print("Failed: ");
      zoneProcessor.printNameTo(SERIAL_PORT_MONITOR);
      SERIAL_PORT_MONITOR.print(": year=");
      SERIAL_PORT_MONITOR.print(year);
      SERIAL_PORT_MONITOR.print(": ");
      SERIAL_PORT_MONITOR.println(msg);
      failTestNow();
    }
};

testF(TransitionValidation, allZones) {
  for (uint16_t i = 0; i < zonedbx::kZoneRegistrySize; i++) {
    const extended::ZoneInfo* info = zoneRegistrar.getZoneInfoForIndex(i);
    zoneProcessor.setZoneKey((uintptr_t) info);

    // Loop from ZoneContext::startYear to ZoneContext::untilYear, in 100 years
    // chunks, because time zone processing is valid over an interval of about
    // 130 years. For each chunk, the currentEpochYear() is reset to an epoch
    // year that is in the middle of each 100-year chunk.
    for (
        int16_t startYear = zonedbx::kZoneContext.startYear;
        startYear < zonedbx::kZoneContext.untilYear;
        startYear += 100) {

      int16_t epochYear = startYear + 50;
      int16_t untilYear = min(
          (int16_t) (epochYear + 50),
          zonedbx::kZoneContext.untilYear);

      testing::EpochYearContext context(epochYear);
      zoneProcessor.resetTransitionCache();

      // FIXME: If a failure is detected, then this function returns early. The
      // currentEpochYear() is guaranteed to be restored through the destructor
      // of the 'context` object, but the cache invalidation clean up is
      // skipped. Most likely this won't cause problems with the rest of the
      // unit tests because the `zoneProcessor` will likely be set to a
      // different year. However, there is a small chance of failure. The proper
      // solution is to create a custom RAII context class to perform the
      // resetTransitionCache() clean up, but I'm too lazy right now.
      assertNoFatalFailure(validateZone(startYear, untilYear));
    }
  }

  zoneProcessor.resetTransitionCache();
}

//----------------------------------------------------------------------------
// Verify Transitions for Europe/Lisbon in 1992. That is the only zone/year
// where the previous ExtendedZoneProcessor algorithm failed, with a duplicate
// Transition. The default zonedbx/ database spans from 2000 until 2050, so we
// have to manually copy the ZoneInfo data for Europe/Lisbon.
//---------------------------------------------------------------------------

testF(TransitionValidation, lisbon1992) {
  zoneProcessor.setZoneKey((uintptr_t) &zonedbxtest::kZoneEurope_Lisbon);
  assertNoFatalFailure(validateZone(
      zonedbxtest::kZoneContext.startYear,
      zonedbxtest::kZoneContext.untilYear));
}

//----------------------------------------------------------------------------

void setup() {
#if ! defined(EPOXY_DUINO)
  delay(1000); // wait to prevent garbage on SERIAL_PORT_MONITOR
#endif
  SERIAL_PORT_MONITOR.begin(115200);
  while (!SERIAL_PORT_MONITOR); // Leonardo/Micro
}

void loop() {
  aunit::TestRunner::run();
}
