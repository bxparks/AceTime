#line 2 "ExtendedTransitionValidationTest.ino"

#include <Arduino.h>
#include <AUnit.h>
#include <AceTime.h>
#include <ace_time/testing/EpochYearContext.h>
#include <testingzonedbx/zone_infos.h>

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
 * This must use the real zonedbx database, not the testingzonedbx database,
 * because we are validating every zone in the IANA TZ database.
 */
class ExtendedTransitionValidation : public aunit::TestOnce {
  public:
    void validateZone(const extended::Info::ZoneInfo* zoneInfo) {
      zoneProcessor.setZoneKey((uintptr_t) zoneInfo);
      auto zoneInfoBroker = extended::Info::ZoneInfoBroker(zoneInfo);
      auto zoneContextBroker = zoneInfoBroker.zoneContext();

      // Loop from ZoneContext::startYear to ZoneContext::untilYear, in 100
      // years chunks, because time zone processing is valid over an interval of
      // about 130 years. For each chunk, the currentEpochYear() is reset to an
      // epoch year that is in the middle of each 100-year chunk.
    #if defined(EPOXY_DUINO)
      // On desktop machines, we can cover 2000 to year 10000.
      for (int16_t startYear = zoneContextBroker.startYear();
          startYear < zoneContextBroker.untilYear();
          startYear += 100) {
    #else
      // On slow microcontrollers, let's check only 2000 to 2100.
      for (int16_t startYear = 2000; startYear < 2100; startYear += 100) {
    #endif

        int16_t epochYear = startYear + 50;
        int16_t untilYear = min(
            (int16_t) (epochYear + 50),
            zoneContextBroker.untilYear());

        testing::EpochYearContext context(epochYear);
        assertNoFatalFailure(validateZone(startYear, untilYear));
      }
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
        // Note: (end-start) is supposed to return an integer of type `ssize_t`,
        // but that type is not defined on AVR g++.
        assertMore(int(end - start), 0);

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

  SERIAL_PORT_MONITOR.print("Validating zones (one per dot): ");
  for (uint16_t i = 0; i < zonedbx::kZoneRegistrySize; i++) {
    const extended::Info::ZoneInfo* zoneInfo =
        zoneRegistrar.getZoneInfoForIndex(i);
    SERIAL_PORT_MONITOR.print(".");
    validateZone(zoneInfo);
  }
  SERIAL_PORT_MONITOR.println();
}

// Verify Transitions for Europe/Lisbon in 1992 using the
// testingzonedbx::kZoneEurope_Lisbon entry which contains entries from 1980 to
// 10000. Lisbon in 1992 was the only combo where the previous
// ExtendedZoneProcessor algorithm failed, with a duplicate Transition.
//
// This uses the testingzonedbx database, instead of the production zonedbx
// database, because we need to test year 1992 and the production zonedbx starts
// at year 2000.
testF(ExtendedTransitionValidation, lisbon1992) {
  assertNoFatalFailure(validateZone(&testingzonedbx::kZoneEurope_Lisbon));
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
