#line 2 "ExtendedZoneProcessorMoreTest.ino"

// Spillovers from ExtendedZoneProcessorTest.ino after it became too big for a
// Arduino Pro Micro.

#include <AUnit.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time;
using namespace ace_time::zonedbx;

// --------------------------------------------------------------------------
// Test public methods
// --------------------------------------------------------------------------

// https://www.timeanddate.com/time/zone/usa/los-angeles
test(ExtendedZoneProcessorTest, kZoneAmerica_Los_Angeles) {
  ExtendedZoneProcessor zoneProcessor(&zonedbx::kZoneAmerica_Los_Angeles);
  OffsetDateTime dt;
  acetime_t epochSeconds;

  dt = OffsetDateTime::forComponents(2018, 3, 11, 1, 59, 59,
      TimeOffset::forHours(-8));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-8*60, zoneProcessor.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("PST", zoneProcessor.getAbbrev(epochSeconds));
  assertTrue(zoneProcessor.getDeltaOffset(epochSeconds).isZero());

  dt = OffsetDateTime::forComponents(2018, 3, 11, 2, 0, 0,
      TimeOffset::forHours(-8));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-7*60, zoneProcessor.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("PDT", zoneProcessor.getAbbrev(epochSeconds));
  assertFalse(zoneProcessor.getDeltaOffset(epochSeconds).isZero());

  dt = OffsetDateTime::forComponents(2018, 11, 4, 1, 0, 0,
      TimeOffset::forHours(-7));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-7*60, zoneProcessor.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("PDT", zoneProcessor.getAbbrev(epochSeconds));
  assertFalse(zoneProcessor.getDeltaOffset(epochSeconds).isZero());

  dt = OffsetDateTime::forComponents(2018, 11, 4, 1, 59, 59,
      TimeOffset::forHours(-7));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-7*60, zoneProcessor.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("PDT", zoneProcessor.getAbbrev(epochSeconds));
  assertFalse(zoneProcessor.getDeltaOffset(epochSeconds).isZero());

  dt = OffsetDateTime::forComponents(2018, 11, 4, 2, 0, 0,
      TimeOffset::forHours(-7));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-8*60, zoneProcessor.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("PST", zoneProcessor.getAbbrev(epochSeconds));
  assertTrue(zoneProcessor.getDeltaOffset(epochSeconds).isZero());
}

test(ExtendedZoneProcessorTest, kZoneAmerica_Los_Angeles_outOfBounds) {
  ExtendedZoneProcessor zoneProcessor(&zonedbx::kZoneAmerica_Los_Angeles);
  OffsetDateTime dt;
  acetime_t epochSeconds;

  assertEqual(2000, zonedbx::kZoneAmerica_Los_Angeles.zoneContext->startYear);
  assertEqual(2050, zonedbx::kZoneAmerica_Los_Angeles.zoneContext->untilYear);

  dt = OffsetDateTime::forComponents(1998, 3, 11, 1, 59, 59,
      TimeOffset::forHours(-8));
  epochSeconds = dt.toEpochSeconds();
  assertTrue(zoneProcessor.getUtcOffset(epochSeconds).isError());
  assertTrue(zoneProcessor.getDeltaOffset(epochSeconds).isError());
  assertEqual("", zoneProcessor.getAbbrev(epochSeconds));

  dt = OffsetDateTime::forComponents(2051, 2, 1, 1, 0, 0,
      TimeOffset::forHours(-8));
  epochSeconds = dt.toEpochSeconds();
  assertTrue(zoneProcessor.getUtcOffset(epochSeconds).isError());
  assertTrue(zoneProcessor.getDeltaOffset(epochSeconds).isError());
  assertEqual("", zoneProcessor.getAbbrev(epochSeconds));
}

// --------------------------------------------------------------------------

void setup() {
#if ! defined(UNIX_HOST_DUINO)
  delay(1000); // wait to prevent garbage on SERIAL_PORT_MONITOR
#endif
  SERIAL_PORT_MONITOR.begin(115200);
  while(!SERIAL_PORT_MONITOR); // for the Arduino Leonardo/Micro only
}

void loop() {
  TestRunner::run();
}
