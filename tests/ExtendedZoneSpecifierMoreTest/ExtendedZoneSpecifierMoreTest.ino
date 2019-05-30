#line 2 "ExtendedZoneSpecifierTest2.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time;
using namespace ace_time::zonedbx;

// --------------------------------------------------------------------------
// Test public methods
// --------------------------------------------------------------------------

// https://www.timeanddate.com/time/zone/usa/los-angeles
test(ExtendedZoneSpecifierTest, kZoneAmerica_Los_Angeles) {
  ExtendedZoneSpecifier zoneSpecifier(&zonedbx::kZoneAmerica_Los_Angeles);
  OffsetDateTime dt;
  acetime_t epochSeconds;

  dt = OffsetDateTime::forComponents(2018, 3, 11, 1, 59, 59,
      TimeOffset::forHour(-8));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-8*60, zoneSpecifier.getTimeOffset(epochSeconds).toMinutes());
  assertEqual("PST", zoneSpecifier.getAbbrev(epochSeconds));
  assertTrue(zoneSpecifier.getDeltaOffset(epochSeconds).isZero());

  dt = OffsetDateTime::forComponents(2018, 3, 11, 2, 0, 0,
      TimeOffset::forHour(-8));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-7*60, zoneSpecifier.getTimeOffset(epochSeconds).toMinutes());
  assertEqual("PDT", zoneSpecifier.getAbbrev(epochSeconds));
  assertFalse(zoneSpecifier.getDeltaOffset(epochSeconds).isZero());

  dt = OffsetDateTime::forComponents(2018, 11, 4, 1, 0, 0,
      TimeOffset::forHour(-7));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-7*60, zoneSpecifier.getTimeOffset(epochSeconds).toMinutes());
  assertEqual("PDT", zoneSpecifier.getAbbrev(epochSeconds));
  assertFalse(zoneSpecifier.getDeltaOffset(epochSeconds).isZero());

  dt = OffsetDateTime::forComponents(2018, 11, 4, 1, 59, 59,
      TimeOffset::forHour(-7));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-7*60, zoneSpecifier.getTimeOffset(epochSeconds).toMinutes());
  assertEqual("PDT", zoneSpecifier.getAbbrev(epochSeconds));
  assertFalse(zoneSpecifier.getDeltaOffset(epochSeconds).isZero());

  dt = OffsetDateTime::forComponents(2018, 11, 4, 2, 0, 0,
      TimeOffset::forHour(-7));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-8*60, zoneSpecifier.getTimeOffset(epochSeconds).toMinutes());
  assertEqual("PST", zoneSpecifier.getAbbrev(epochSeconds));
  assertTrue(zoneSpecifier.getDeltaOffset(epochSeconds).isZero());
}

test(ExtendedZoneSpecifierTest, kZoneAmerica_Los_Angeles_outOfBounds) {
  ExtendedZoneSpecifier zoneSpecifier(&zonedbx::kZoneAmerica_Los_Angeles);
  OffsetDateTime dt;
  acetime_t epochSeconds;

  assertEqual(2000, zonedbx::kZoneAmerica_Los_Angeles.zoneContext->startYear);
  assertEqual(2038, zonedbx::kZoneAmerica_Los_Angeles.zoneContext->untilYear);

  dt = OffsetDateTime::forComponents(1998, 3, 11, 1, 59, 59,
      TimeOffset::forHour(-8));
  epochSeconds = dt.toEpochSeconds();
  assertTrue(zoneSpecifier.getTimeOffset(epochSeconds).isError());
  assertTrue(zoneSpecifier.getDeltaOffset(epochSeconds).isError());
  assertEqual("", zoneSpecifier.getAbbrev(epochSeconds));

  dt = OffsetDateTime::forComponents(2039, 2, 1, 1, 0, 0,
      TimeOffset::forHour(-8));
  epochSeconds = dt.toEpochSeconds();
  assertTrue(zoneSpecifier.getTimeOffset(epochSeconds).isError());
  assertTrue(zoneSpecifier.getDeltaOffset(epochSeconds).isError());
  assertEqual("", zoneSpecifier.getAbbrev(epochSeconds));
}

// --------------------------------------------------------------------------

void setup() {
#if defined(ARDUINO)
  delay(1000); // wait for stability on some boards to prevent garbage Serial
#endif
  Serial.begin(115200); // ESP8266 default of 74880 not supported on Linux
  while(!Serial); // for the Arduino Leonardo/Micro only
}

void loop() {
  TestRunner::run();
}
