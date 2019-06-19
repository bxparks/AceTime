#line 2 "ZonedDateTimeExtendedTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time;

// --------------------------------------------------------------------------
// ZonedDateTime + ExtendedZoneSpecifier
// --------------------------------------------------------------------------

test(ZonedDateTimeTest_Extended, forComponents_isError) {
  ExtendedZoneSpecifier zoneSpecifier(&zonedbx::kZoneAmerica_Los_Angeles);
  TimeZone tz = TimeZone::forZoneSpecifier(&zoneSpecifier);

  // outside [2000, 2050) range, should generate error
  ZonedDateTime dt = ZonedDateTime::forComponents(1998, 3, 11, 1, 59, 59, tz);
  assertTrue(dt.isError());
  dt = ZonedDateTime::forComponents(2051, 3, 11, 1, 59, 59, tz);
  assertTrue(dt.isError());
}

test(ZonedDateTimeTest_Extended, forComponents_beforeDst) {
  ExtendedZoneSpecifier zoneSpecifier(&zonedbx::kZoneAmerica_Los_Angeles);
  TimeZone tz = TimeZone::forZoneSpecifier(&zoneSpecifier);
  ZonedDateTime dt = ZonedDateTime::forComponents(2018, 3, 11, 1, 59, 59, tz);

  TimeOffset expectedOffset = TimeOffset::forHour(-8);
  OffsetDateTime otz = OffsetDateTime::forComponents(2018, 3, 11, 1, 59, 59,
    expectedOffset);

  assertEqual(otz.toEpochSeconds(), dt.toEpochSeconds());
}

test(ZonedDateTimeTest_Extended, forComponents_inDstGap) {
  ExtendedZoneSpecifier zoneSpecifier(&zonedbx::kZoneAmerica_Los_Angeles);
  TimeZone tz = TimeZone::forZoneSpecifier(&zoneSpecifier);
  ZonedDateTime dt = ZonedDateTime::forComponents(2018, 3, 11, 2, 0, 1, tz);

  // The expected TimeOffset in the gap is the previous timeOffset, i.e. the
  // most recent matching Transition.
  TimeOffset expectedOffset = TimeOffset::forHour(-8);
  OffsetDateTime odt = OffsetDateTime::forComponents(2018, 3, 11, 2, 0, 1,
      expectedOffset);

  assertEqual(odt.toEpochSeconds(), dt.toEpochSeconds());
}

test(ZonedDateTimeTest_Extended, forComponents_inDst) {
  ExtendedZoneSpecifier zoneSpecifier(&zonedbx::kZoneAmerica_Los_Angeles);
  TimeZone tz = TimeZone::forZoneSpecifier(&zoneSpecifier);
  ZonedDateTime dt = ZonedDateTime::forComponents(2018, 3, 11, 3, 0, 1, tz);

  TimeOffset expectedOffset = TimeOffset::forHour(-7);
  OffsetDateTime odt = OffsetDateTime::forComponents(2018, 3, 11, 3, 0, 1,
      expectedOffset);

  assertEqual(odt.toEpochSeconds(), dt.toEpochSeconds());
}

test(ZonedDateTimeTest_Extended, forComponents_inOverlap) {
  ExtendedZoneSpecifier zoneSpecifier(&zonedbx::kZoneAmerica_Los_Angeles);
  TimeZone tz = TimeZone::forZoneSpecifier(&zoneSpecifier);
  ZonedDateTime dt = ZonedDateTime::forComponents(2018, 11, 4, 1, 30, 0, tz);

  // When 2 UTC offsets can be valid, the algorithm picks the later one,
  // i.e. the most recent matching Transition.
  TimeOffset expectedOffset = TimeOffset::forHour(-8);
  OffsetDateTime odt = OffsetDateTime::forComponents(2018, 11, 4, 1, 30, 0,
      expectedOffset);

  assertEqual(odt.toEpochSeconds(), dt.toEpochSeconds());
}

test(ZonedDateTimeTest_Extended, forComponents_afterOverlap) {
  ExtendedZoneSpecifier zoneSpecifier(&zonedbx::kZoneAmerica_Los_Angeles);
  TimeZone tz = TimeZone::forZoneSpecifier(&zoneSpecifier);
  ZonedDateTime dt = ZonedDateTime::forComponents(2018, 11, 4, 2, 30, 0, tz);

  TimeOffset expectedOffset = TimeOffset::forHour(-8);
  OffsetDateTime odt = OffsetDateTime::forComponents(2018, 11, 4, 2, 30, 0,
      expectedOffset);

  assertEqual(odt.toEpochSeconds(), dt.toEpochSeconds());
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
