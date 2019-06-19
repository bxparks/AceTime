#line 2 "ZonedDateTimeBasicTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time;

// --------------------------------------------------------------------------
// ZonedDateTime + BasicZoneSpecifier
// --------------------------------------------------------------------------

test(ZonedDateTimeTest_Basic, forComponents_isError) {
  BasicZoneSpecifier zoneSpecifier(&zonedb::kZoneAmerica_Los_Angeles);
  TimeZone tz = TimeZone::forZoneSpecifier(&zoneSpecifier);

  // outside [2000, 2050) range, should generate error
  ZonedDateTime dt = ZonedDateTime::forComponents(1998, 3, 11, 1, 59, 59, tz);
  assertTrue(dt.isError());
  dt = ZonedDateTime::forComponents(2051, 3, 11, 1, 59, 59, tz);
  assertTrue(dt.isError());
}

test(ZonedDateTimeTest_Basic, forComponents_beforeDst) {
  BasicZoneSpecifier zoneSpecifier(&zonedb::kZoneAmerica_Los_Angeles);
  TimeZone tz = TimeZone::forZoneSpecifier(&zoneSpecifier);
  ZonedDateTime dt = ZonedDateTime::forComponents(2018, 3, 11, 1, 59, 59, tz);

  TimeOffset pst = TimeOffset::forHour(-8);
  OffsetDateTime otz = OffsetDateTime::forComponents(2018, 3, 11, 1, 59, 59,
    pst);

  assertEqual(otz.toEpochSeconds(), dt.toEpochSeconds());
}

test(ZonedDateTimeTest_Basic, forComponents_inDstGap) {
  BasicZoneSpecifier zoneSpecifier(&zonedb::kZoneAmerica_Los_Angeles);
  TimeZone tz = TimeZone::forZoneSpecifier(&zoneSpecifier);
  ZonedDateTime dt = ZonedDateTime::forComponents(2018, 3, 11, 2, 0, 1, tz);

  TimeOffset pdt = TimeOffset::forHour(-7);
  OffsetDateTime odt = OffsetDateTime::forComponents(2018, 3, 11, 2, 0, 1, pdt);

  assertEqual(odt.toEpochSeconds(), dt.toEpochSeconds());
}

test(ZonedDateTimeTest_Basic, forComponents_inDst) {
  BasicZoneSpecifier zoneSpecifier(&zonedb::kZoneAmerica_Los_Angeles);
  TimeZone tz = TimeZone::forZoneSpecifier(&zoneSpecifier);
  ZonedDateTime dt = ZonedDateTime::forComponents(2018, 3, 11, 3, 0, 1, tz);

  TimeOffset pdt = TimeOffset::forHour(-7);
  OffsetDateTime odt = OffsetDateTime::forComponents(2018, 3, 11, 3, 0, 1, pdt);

  assertEqual(odt.toEpochSeconds(), dt.toEpochSeconds());
}

test(ZonedDateTimeTest_Basic, forComponents_beforeStd) {
  BasicZoneSpecifier zoneSpecifier(&zonedb::kZoneAmerica_Los_Angeles);
  TimeZone tz = TimeZone::forZoneSpecifier(&zoneSpecifier);
  ZonedDateTime dt = ZonedDateTime::forComponents(2018, 11, 4, 0, 59, 59, tz);

  TimeOffset pdt = TimeOffset::forHour(-7);
  OffsetDateTime odt = OffsetDateTime::forComponents(2018, 11, 4, 0, 59, 59,
      pdt);

  assertEqual(odt.toEpochSeconds(), dt.toEpochSeconds());
}

test(ZonedDateTimeTest_Basic, forComponents_inOverlap) {
  BasicZoneSpecifier zoneSpecifier(&zonedb::kZoneAmerica_Los_Angeles);
  TimeZone tz = TimeZone::forZoneSpecifier(&zoneSpecifier);
  ZonedDateTime dt = ZonedDateTime::forComponents(
      2018, 11, 4, 1, 0, 1, tz); // ambiguous

  TimeOffset pdt = TimeOffset::forHour(-8);
  OffsetDateTime odt = OffsetDateTime::forComponents(2018, 11, 4, 1, 0, 1, pdt);

  assertEqual(odt.toEpochSeconds(), dt.toEpochSeconds());
}

test(ZonedDateTimeTest_Basic, forComponents_afterOverlap) {
  BasicZoneSpecifier zoneSpecifier(&zonedb::kZoneAmerica_Los_Angeles);
  TimeZone tz = TimeZone::forZoneSpecifier(&zoneSpecifier);
  ZonedDateTime dt = ZonedDateTime::forComponents(
      2018, 11, 4, 2, 0, 1, tz); // ambiguous

  TimeOffset pdt = TimeOffset::forHour(-8);
  OffsetDateTime odt = OffsetDateTime::forComponents(2018, 11, 4, 2, 0, 1, pdt);

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
