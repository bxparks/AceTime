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

  // 01:59 should resolve to 01:59-08:00
  auto dt = ZonedDateTime::forComponents(2018, 3, 11, 1, 59, 0, tz);
  assertEqual(TimeOffset::forHour(-8).toMinutes(), dt.timeOffset().toMinutes());
  auto expected = LocalDateTime::forComponents(2018, 3, 11, 1, 59, 0);
  assertTrue(expected == dt.localDateTime());
}

test(ZonedDateTimeTest_Extended, forComponents_inDstGap) {
  ExtendedZoneSpecifier zoneSpecifier(&zonedbx::kZoneAmerica_Los_Angeles);
  TimeZone tz = TimeZone::forZoneSpecifier(&zoneSpecifier);

  // 02:01 doesn't exist. The expected TimeOffset in the gap is the previous
  // timeOffset, i.e. the most recent matching Transition, so this is
  // interpreted as 02:01-08:00 which gets normalized to 03:01-07:00.
  auto dt = ZonedDateTime::forComponents(2018, 3, 11, 2, 1, 0, tz);
  assertEqual(TimeOffset::forHour(-7).toMinutes(), dt.timeOffset().toMinutes());
  auto expected = LocalDateTime::forComponents(2018, 3, 11, 3, 1, 0);
  assertTrue(expected == dt.localDateTime());
}

test(ZonedDateTimeTest_Extended, forComponents_inDst) {
  ExtendedZoneSpecifier zoneSpecifier(&zonedbx::kZoneAmerica_Los_Angeles);
  TimeZone tz = TimeZone::forZoneSpecifier(&zoneSpecifier);

  // 03:01 should resolve to 03:01-07:00.
  auto dt = ZonedDateTime::forComponents(2018, 3, 11, 3, 1, 0, tz);
  assertEqual(TimeOffset::forHour(-7).toMinutes(), dt.timeOffset().toMinutes());
  auto expected = LocalDateTime::forComponents(2018, 3, 11, 3, 1, 0);
  assertTrue(expected == dt.localDateTime());
}

test(ZonedDateTimeTest_Extended, forComponents_beforeStd) {
  ExtendedZoneSpecifier zoneSpecifier(&zonedbx::kZoneAmerica_Los_Angeles);
  TimeZone tz = TimeZone::forZoneSpecifier(&zoneSpecifier);

  // 00:59 is an hour before the DST->STD transition, so should return
  // 00:59-07:00.
  auto dt = ZonedDateTime::forComponents(2018, 11, 4, 0, 59, 0, tz);
  assertEqual(TimeOffset::forHour(-7).toMinutes(), dt.timeOffset().toMinutes());
  auto expected = LocalDateTime::forComponents(2018, 11, 4, 0, 59, 0);
  assertTrue(expected == dt.localDateTime());
}

test(ZonedDateTimeTest_Extended, forComponents_inOverlap) {
  ExtendedZoneSpecifier zoneSpecifier(&zonedbx::kZoneAmerica_Los_Angeles);
  TimeZone tz = TimeZone::forZoneSpecifier(&zoneSpecifier);

  // There were two instances of 01:01. The algorithm picks the later offset,
  // i.e. the most recent matching Transition, so should resolve to 01:01-08:00.
  auto dt = ZonedDateTime::forComponents(2018, 11, 4, 1, 1, 0, tz);
  assertEqual(TimeOffset::forHour(-8).toMinutes(), dt.timeOffset().toMinutes());
  auto expected = LocalDateTime::forComponents(2018, 11, 4, 1, 1, 0);
  assertTrue(expected == dt.localDateTime());
}

test(ZonedDateTimeTest_Extended, forComponents_afterOverlap) {
  ExtendedZoneSpecifier zoneSpecifier(&zonedbx::kZoneAmerica_Los_Angeles);
  TimeZone tz = TimeZone::forZoneSpecifier(&zoneSpecifier);

  // 02:01 should resolve to 02:01-08:00
  auto dt = ZonedDateTime::forComponents(2018, 11, 4, 2, 1, 0, tz);
  assertEqual(TimeOffset::forHour(-8).toMinutes(), dt.timeOffset().toMinutes());
  auto expected = LocalDateTime::forComponents(2018, 11, 4, 2, 1, 0);
  assertTrue(expected == dt.localDateTime());
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
