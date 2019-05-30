#line 2 "ZonedDateTimeTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time;

// --------------------------------------------------------------------------
// ZonedDateTime + ManualZoneSpecifier
// --------------------------------------------------------------------------

// Check that ZonedDateTime with ManualZoneSpecifier agrees with simpler
// OffsetDateTime.
test(ZonedDateTimeTest_Manual, agreesWithOffsetDateTime) {
  ManualZoneSpecifier zoneSpecifier(TimeOffset::forHour(-8), false);
  TimeZone tz = TimeZone::forZoneSpecifier(&zoneSpecifier);
  ZonedDateTime dt = ZonedDateTime::forComponents(2018, 3, 11, 1, 59, 59, tz);

  OffsetDateTime otz = OffsetDateTime::forComponents(2018, 3, 11, 1, 59, 59,
      TimeOffset::forHour(-8));

  assertEqual(otz.toEpochSeconds(), dt.toEpochSeconds());
}

test(ZonedDateTimeTest_Manual, forComponents) {
  ZonedDateTime dt;

  // 1931-12-13 20:45:52Z, smalltest datetime using int32_t from AceTime Epoch.
  // Let's use +1 of that since INT_MIN will be used to indicate an error.
  dt = ZonedDateTime::forComponents(1931, 12, 13, 20, 45, 53);
  assertEqual((acetime_t) -24856, dt.toEpochDays());
  assertEqual((acetime_t) -13899, dt.toUnixDays());
  assertEqual((acetime_t) (INT32_MIN + 1), dt.toEpochSeconds());
  assertEqual(LocalDate::kSunday, dt.dayOfWeek());

  // 2000-01-01 00:00:00Z Saturday
  dt = ZonedDateTime::forComponents(2000, 1, 1, 0, 0, 0);
  assertEqual((acetime_t) 0, dt.toEpochDays());
  assertEqual((acetime_t) 10957, dt.toUnixDays());
  assertEqual((acetime_t) 0, dt.toEpochSeconds());
  assertEqual(LocalDate::kSaturday, dt.dayOfWeek());

  // 2000-01-02 00:00:00Z Sunday
  dt = ZonedDateTime::forComponents(2000, 1, 2, 0, 0, 0);
  assertEqual((acetime_t) 1, dt.toEpochDays());
  assertEqual((acetime_t) 10958, dt.toUnixDays());
  assertEqual((acetime_t) 86400, dt.toEpochSeconds());
  assertEqual(LocalDate::kSunday, dt.dayOfWeek());

  // 2000-02-29 00:00:00Z Tuesday
  dt = ZonedDateTime::forComponents(2000, 2, 29, 0, 0, 0);
  assertEqual((acetime_t) 59, dt.toEpochDays());
  assertEqual((acetime_t) 11016, dt.toUnixDays());
  assertEqual((acetime_t) 86400 * 59, dt.toEpochSeconds());
  assertEqual(LocalDate::kTuesday, dt.dayOfWeek());

  // 2018-01-01 00:00:00Z Monday
  dt = ZonedDateTime::forComponents(2018, 1, 1, 0, 0, 0);
  assertEqual((acetime_t) 6575, dt.toEpochDays());
  assertEqual((acetime_t) 17532, dt.toUnixDays());
  assertEqual(6575 * (acetime_t) 86400, dt.toEpochSeconds());
  assertEqual(LocalDate::kMonday, dt.dayOfWeek());

  // 2018-01-01 00:00:00+00:15 Monday
  ManualZoneSpecifier zoneSpecifier(TimeOffset::forMinutes(15), false);
  dt = ZonedDateTime::forComponents(2018, 1, 1, 0, 0, 0,
      TimeZone::forZoneSpecifier(&zoneSpecifier));
  assertEqual((acetime_t) 6574, dt.toEpochDays());
  assertEqual((acetime_t) 17531, dt.toUnixDays());
  assertEqual(6575 * (acetime_t) 86400 - 15*60, dt.toEpochSeconds());
  assertEqual(LocalDate::kMonday, dt.dayOfWeek());

  // 2038-01-19 03:14:07Z (largest value using Unix Epoch)
  dt = ZonedDateTime::forComponents(2038, 1, 19, 3, 14, 7);
  assertEqual((acetime_t) 13898, dt.toEpochDays());
  assertEqual((acetime_t) 24855, dt.toUnixDays());
  assertEqual((acetime_t) 1200798847, dt.toEpochSeconds());
  assertEqual(LocalDate::kTuesday, dt.dayOfWeek());

  // 2068-01-19 03:14:06Z (largest value for AceTime Epoch).
  dt = ZonedDateTime::forComponents(2068, 1, 19, 3, 14, 7);
  assertEqual((acetime_t) 24855, dt.toEpochDays());
  assertEqual((acetime_t) 35812, dt.toUnixDays());
  assertEqual((acetime_t) (INT32_MAX), dt.toEpochSeconds());
  assertEqual(LocalDate::kThursday, dt.dayOfWeek());
}

test(ZonedDateTimeTest_Manual, toAndForUnixSeconds) {
  ZonedDateTime dt;
  ZonedDateTime udt;

  // 1931-12-13 20:45:52Z, smalltest datetime using int32_t from AceTime Epoch.
  // Let's use +1 of that since INT_MIN will be used to indicate an error.
  dt = ZonedDateTime::forComponents(1931, 12, 13, 20, 45, 53);
  assertEqual((acetime_t) -1200798847, dt.toUnixSeconds());
  udt = ZonedDateTime::forUnixSeconds(dt.toUnixSeconds());
  assertTrue(dt == udt);

  // 1970-01-01 00:00:00Z
  dt = ZonedDateTime::forComponents(1970, 1, 1, 0, 0, 0);
  assertEqual((acetime_t) 0, dt.toUnixSeconds());
  udt = ZonedDateTime::forUnixSeconds(dt.toUnixSeconds());
  assertTrue(dt == udt);

  // 2000-01-01 00:00:00Z
  dt = ZonedDateTime::forComponents(2000, 1, 1, 0, 0, 0);
  assertEqual((acetime_t) 946684800, dt.toUnixSeconds());
  udt = ZonedDateTime::forUnixSeconds(dt.toUnixSeconds());
  assertTrue(dt == udt);


  // 2018-01-01 00:00:00Z
  dt = ZonedDateTime::forComponents(2018, 1, 1, 0, 0, 0);
  assertEqual((acetime_t) 1514764800, dt.toUnixSeconds());
  udt = ZonedDateTime::forUnixSeconds(dt.toUnixSeconds());
  assertTrue(dt == udt);

  // 2018-08-30T06:45:01-07:00
  ManualZoneSpecifier zoneSpecifier(TimeOffset::forHour(-7), false);
  TimeZone tz = TimeZone::forZoneSpecifier(&zoneSpecifier);
  dt = ZonedDateTime::forComponents(2018, 8, 30, 6, 45, 1, tz);
  assertEqual((acetime_t) 1535636701, dt.toUnixSeconds());
  udt = ZonedDateTime::forUnixSeconds(dt.toUnixSeconds(), tz);
  assertTrue(dt == udt);

  // 2038-01-19 03:14:06Z (largest value - 1 using Unix Epoch)
  dt = ZonedDateTime::forComponents(2038, 1, 19, 3, 14, 6);
  assertEqual((acetime_t) (INT32_MAX - 1), dt.toUnixSeconds());
  udt = ZonedDateTime::forUnixSeconds(dt.toUnixSeconds());
  assertTrue(dt == udt);
}

test(ZonedDateTimeTest_Manual, convertToTimeZone) {
  ManualZoneSpecifier stdSpec(TimeOffset::forHour(-8), false);
  TimeZone stdTz = TimeZone::forZoneSpecifier(&stdSpec);
  ZonedDateTime std = ZonedDateTime::forComponents(
      2018, 3, 11, 1, 59, 59, stdTz);
  acetime_t stdEpochSeconds = std.toEpochSeconds();

  ManualZoneSpecifier dstSpec(stdSpec);
  dstSpec.isDst(true);
  TimeZone dstTz = TimeZone::forZoneSpecifier(&dstSpec);
  ZonedDateTime dst = std.convertToTimeZone(dstTz);
  acetime_t dstEpochSeconds = dst.toEpochSeconds();

  assertEqual(stdEpochSeconds, dstEpochSeconds);

  assertEqual((int16_t) 2018, dst.year());
  assertEqual(3, dst.month());
  assertEqual(11, dst.day());
  assertEqual(2, dst.hour());
  assertEqual(59, dst.minute());
  assertEqual(59, dst.second());
  assertEqual(-7*60, dst.timeZone().getTimeOffset(stdEpochSeconds).toMinutes());
}

// --------------------------------------------------------------------------
// ZonedDateTime + BasicZoneSpecifier
// --------------------------------------------------------------------------

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
// ZonedDateTime + ExtendedZoneSpecifier
// --------------------------------------------------------------------------

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
// forDateString()
// --------------------------------------------------------------------------

test(ZonedDateTimeTest, forDateString) {
  // exact ISO8601 format
  auto dt = ZonedDateTime::forDateString(F("2018-08-31T13:48:01-07:00"));
  assertFalse(dt.isError());
  assertEqual((int16_t) 2018, dt.year());
  assertEqual(18, dt.yearTiny());
  assertEqual(8, dt.month());
  assertEqual(31, dt.day());
  assertEqual(13, dt.hour());
  assertEqual(48, dt.minute());
  assertEqual(1, dt.second());
  assertEqual(-7*60, dt.timeOffset().toMinutes());
  assertEqual(LocalDate::kFriday, dt.dayOfWeek());

  // parser does not care about most separators, this may change in the future
  dt = ZonedDateTime::forDateString(F("2018/08/31 13#48#01+07#00"));
  assertFalse(dt.isError());
  assertEqual((int16_t) 2018, dt.year());
  assertEqual(18, dt.yearTiny());
  assertEqual(8, dt.month());
  assertEqual(31, dt.day());
  assertEqual(13, dt.hour());
  assertEqual(48, dt.minute());
  assertEqual(1, dt.second());
  assertEqual(7*60, dt.timeOffset().toMinutes());
  assertEqual(LocalDate::kFriday, dt.dayOfWeek());
}

test(ZonedDateTimeTest, forDateString_errors) {
  // empty string, too short
  auto dt = ZonedDateTime::forDateString("");
  assertTrue(dt.isError());

  // not enough components
  dt = ZonedDateTime::forDateString(F("2018-08-31"));
  assertTrue(dt.isError());

  // too long
  dt = ZonedDateTime::forDateString(F("2018-08-31T13:48:01-07:00X"));
  assertTrue(dt.isError());

  // too short
  dt = ZonedDateTime::forDateString(F("2018-08-31T13:48:01-07:0"));
  assertTrue(dt.isError());

  // missing UTC
  dt = ZonedDateTime::forDateString(F("2018-08-31T13:48:01"));
  assertTrue(dt.isError());

  // parser cares about the +/- in front of the UTC offset
  dt = ZonedDateTime::forDateString(F("2018-08-31 13:48:01&07:00"));
  assertTrue(dt.isError());
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
