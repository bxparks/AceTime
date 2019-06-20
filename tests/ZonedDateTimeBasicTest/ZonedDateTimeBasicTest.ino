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
  ZonedDateTime dt = ZonedDateTime::forComponents(1998, 3, 11, 1, 59, 0, tz);
  assertTrue(dt.isError());
  dt = ZonedDateTime::forComponents(2051, 3, 11, 1, 59, 0, tz);
  assertTrue(dt.isError());
}

test(ZonedDateTimeTest_Basic, forComponents_beforeDst) {
  BasicZoneSpecifier zoneSpecifier(&zonedb::kZoneAmerica_Los_Angeles);
  TimeZone tz = TimeZone::forZoneSpecifier(&zoneSpecifier);
  auto dtInput = ZonedDateTime::forComponents(2018, 3, 11, 1, 59, 0, tz);
  auto dt = ZonedDateTime::forEpochSeconds(dtInput.toEpochSeconds(), tz);

  assertEqual(TimeOffset::forHour(-8).toMinutes(), dt.timeOffset().toMinutes());
  auto expected = LocalDateTime::forComponents(2018, 3, 11, 1, 59, 0);
  assertTrue(expected == dt.localDateTime());
}

test(ZonedDateTimeTest_Basic, forComponents_inDstGap) {
  BasicZoneSpecifier zoneSpecifier(&zonedb::kZoneAmerica_Los_Angeles);
  TimeZone tz = TimeZone::forZoneSpecifier(&zoneSpecifier);
  // 2:01am doesn't exist
  auto dtInput = ZonedDateTime::forComponents(2018, 3, 11, 2, 1, 0, tz);
  auto dt = ZonedDateTime::forEpochSeconds(dtInput.toEpochSeconds(), tz);

  // TODO: The time gets shifted back to 1:01am, which is probably not what the
  // user expected, but that's the current result in the gap.
  assertEqual(TimeOffset::forHour(-8).toMinutes(), dt.timeOffset().toMinutes());
  auto expected = LocalDateTime::forComponents(2018, 3, 11, 1, 1, 0);
  assertTrue(expected == dt.localDateTime());
}

test(ZonedDateTimeTest_Basic, forComponents_inDst) {
  BasicZoneSpecifier zoneSpecifier(&zonedb::kZoneAmerica_Los_Angeles);
  TimeZone tz = TimeZone::forZoneSpecifier(&zoneSpecifier);
  auto dtInput = ZonedDateTime::forComponents(2018, 3, 11, 3, 1, 0, tz);
  auto dt = ZonedDateTime::forEpochSeconds(dtInput.toEpochSeconds(), tz);

  assertEqual(TimeOffset::forHour(-7).toMinutes(), dt.timeOffset().toMinutes());
  auto expected = LocalDateTime::forComponents(2018, 3, 11, 3, 1, 0);
  assertTrue(expected == dt.localDateTime());
}

test(ZonedDateTimeTest_Basic, forComponents_beforeStd) {
  BasicZoneSpecifier zoneSpecifier(&zonedb::kZoneAmerica_Los_Angeles);
  TimeZone tz = TimeZone::forZoneSpecifier(&zoneSpecifier);
  auto dtInput = ZonedDateTime::forComponents(2018, 11, 4, 0, 59, 0, tz);
  auto dt = ZonedDateTime::forEpochSeconds(dtInput.toEpochSeconds(), tz);

  assertEqual(TimeOffset::forHour(-7).toMinutes(), dt.timeOffset().toMinutes());
  auto expected = LocalDateTime::forComponents(2018, 11, 4, 0, 59, 0);
  assertTrue(expected == dt.localDateTime());
}

test(ZonedDateTimeTest_Basic, forComponents_inOverlap) {
  BasicZoneSpecifier zoneSpecifier(&zonedb::kZoneAmerica_Los_Angeles);
  TimeZone tz = TimeZone::forZoneSpecifier(&zoneSpecifier);
  // There were two instances of 1:00am.
  auto dtInput = ZonedDateTime::forComponents(2018, 11, 4, 1, 1, 0, tz);
  auto dt = ZonedDateTime::forEpochSeconds(dtInput.toEpochSeconds(), tz);

  // BasicZoneSpecifier chooses the earlier offset.
  assertEqual(TimeOffset::forHour(-8).toMinutes(), dt.timeOffset().toMinutes());
  auto expected = LocalDateTime::forComponents(2018, 11, 4, 1, 1, 0);
  assertTrue(expected == dt.localDateTime());
}

test(ZonedDateTimeTest_Basic, forComponents_afterOverlap) {
  BasicZoneSpecifier zoneSpecifier(&zonedb::kZoneAmerica_Los_Angeles);
  TimeZone tz = TimeZone::forZoneSpecifier(&zoneSpecifier);
  auto dtInput = ZonedDateTime::forComponents(2018, 11, 4, 2, 1, 0, tz);
  auto dt = ZonedDateTime::forEpochSeconds(dtInput.toEpochSeconds(), tz);

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
