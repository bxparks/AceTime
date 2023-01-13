#line 2 "ExtendedZoneProcessorTransitionTest.ino"

#include <Arduino.h>
#include <AUnit.h>
#include <AceTime.h>
#include <ace_time/testing/EpochYearContext.h>
#include <ace_time/testing/tzonedbx/zone_policies.h>
#include <ace_time/testing/tzonedbx/zone_infos.h>

using namespace ace_time;

//----------------------------------------------------------------------------

// This could go inside ExtendedTransitionValidation class, but it consumes a
// fair bit of memory, so let's extract it outside.
ExtendedZoneProcessor zoneProcessor;

//----------------------------------------------------------------------------

/**
 * Check that all Transitions for all years, for all zones in the
 * zonedbx database are:
 *  * sorted with respect to startEpochSeconds
 *  * unique with respect to startEpochSeconds
 *
 * This must use the real zonedbx database, not the testing/tzonedbx database,
 * because we are validating every zone in the IANA TZ database.
 */
class ExtendedTransitionValidation : public aunit::TestOnce {
  public:
    void validateZone(const extended::ZoneInfo* zoneInfo) {
      zoneProcessor.setZoneKey((uintptr_t) zoneInfo);

      // Loop from ZoneContext::startYear to ZoneContext::untilYear, in 100
      // years chunks, because time zone processing is valid over an interval of
      // about 130 years. For each chunk, the currentEpochYear() is reset to an
      // epoch year that is in the middle of each 100-year chunk.
      for (int16_t startYear = zonedbx::kZoneContext.startYear;
          startYear < zonedbx::kZoneContext.untilYear;
          startYear += 100) {

        int16_t epochYear = startYear + 50;
        int16_t untilYear = min(
            (int16_t) (epochYear + 50),
            zonedbx::kZoneContext.untilYear);

        testing::EpochYearContext context(epochYear);
        zoneProcessor.resetTransitionCache();

        // FIXME: If a failure is detected, then this function returns early.
        // The currentEpochYear() is guaranteed to be restored through the
        // destructor of the 'context` object, but the cache invalidation clean
        // up is skipped. Most likely this won't cause problems with the rest of
        // the unit tests because the `zoneProcessor` will likely be set to a
        // different year. However, there is a small chance of failure. The
        // proper solution is to create a custom RAII context class to perform
        // the resetTransitionCache() clean up, but I'm too lazy right now.
        assertNoFatalFailure(validateZone(startYear, untilYear));
      }

      // Perform a final resetTransitionCache() in case a test case failed,
      // which causes the epoch year to be reset to its original value, but
      // the zoneProcessor cache is not automatically reset.
      zoneProcessor.resetTransitionCache();
    }

    // Validate the current zoneProcessor state using the [start, until)
    // interval.
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

        // Verify at least one Transition is created for each zone.
        assertMore(end - start, (ssize_t) 0);

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

//----------------------------------------------------------------------------

// Verify transitions for all zones in the zonedbx::kZoneRegistry.
testF(ExtendedTransitionValidation, allZones) {
  extended::ZoneRegistrar zoneRegistrar(
      zonedbx::kZoneRegistrySize,
      zonedbx::kZoneRegistry);

  for (uint16_t i = 0; i < zonedbx::kZoneRegistrySize; i++) {
    const extended::ZoneInfo* zoneInfo = zoneRegistrar.getZoneInfoForIndex(i);
    validateZone(zoneInfo);
  }
}

// Verify Transitions for Europe/Lisbon in 1992 using the
// tzonedbx::kZoneEurope_Lisbon entry which contains entries from 1980 to 10000.
// Lisbon in 1992 was the only combo where the previous ExtendedZoneProcessor
// algorithm failed, with a duplicate Transition.
//
// This uses the testing/tzonedbx database, instead of the production zonedbx
// database, because we need to test year 1992 and the production zonedbx starts
// at year 2000.
testF(ExtendedTransitionValidation, lisbon1992) {
  assertNoFatalFailure(validateZone(&tzonedbx::kZoneEurope_Lisbon));
}

//----------------------------------------------------------------------------

void setup() {
#if ! defined(EPOXY_DUINO)
  delay(1000); // wait to prevent garbage on SERIAL_PORT_MONITOR
#endif
  SERIAL_PORT_MONITOR.begin(115200);
  while (!SERIAL_PORT_MONITOR); // Leonardo/Micro
#if defined(EPOXY_DUINO)
  SERIAL_PORT_MONITOR.setLineModeUnix();
#endif
}

void loop() {
  aunit::TestRunner::run();
}
