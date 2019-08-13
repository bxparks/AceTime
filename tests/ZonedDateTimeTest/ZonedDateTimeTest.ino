#line 2 "ZonedDateTimeTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time;

// --------------------------------------------------------------------------
// ZonedDateTime + Manual TimeZone
// --------------------------------------------------------------------------

// Check that ZonedDateTime with Manual TimeZone agrees with simpler
// OffsetDateTime.
test(ZonedDateTimeTest_Manual, agreesWithOffsetDateTime) {
  TimeZone tz = TimeZone::forTimeOffset(TimeOffset::forHours(-8));
  ZonedDateTime dt = ZonedDateTime::forComponents(2018, 3, 11, 1, 59, 59, tz);

  OffsetDateTime otz = OffsetDateTime::forComponents(2018, 3, 11, 1, 59, 59,
      TimeOffset::forHours(-8));

  assertEqual(otz.toEpochSeconds(), dt.toEpochSeconds());
}

test(ZonedDateTimeTest_Manual, forComponents) {
  ZonedDateTime dt;

  // 1931-12-13 20:45:52Z, smalltest datetime using int32_t from AceTime Epoch.
  // Let's use +1 of that since INT_MIN will be used to indicate an error.
  dt = ZonedDateTime::forComponents(1931, 12, 13, 20, 45, 53, TimeZone());
  assertEqual((acetime_t) -24856, dt.toEpochDays());
  assertEqual((acetime_t) -13899, dt.toUnixDays());
  assertEqual((acetime_t) (INT32_MIN + 1), dt.toEpochSeconds());
  assertEqual(LocalDate::kSunday, dt.dayOfWeek());

  // 2000-01-01 00:00:00Z Saturday
  dt = ZonedDateTime::forComponents(2000, 1, 1, 0, 0, 0, TimeZone());
  assertEqual((acetime_t) 0, dt.toEpochDays());
  assertEqual((acetime_t) 10957, dt.toUnixDays());
  assertEqual((acetime_t) 0, dt.toEpochSeconds());
  assertEqual(LocalDate::kSaturday, dt.dayOfWeek());

  // 2000-01-02 00:00:00Z Sunday
  dt = ZonedDateTime::forComponents(2000, 1, 2, 0, 0, 0, TimeZone());
  assertEqual((acetime_t) 1, dt.toEpochDays());
  assertEqual((acetime_t) 10958, dt.toUnixDays());
  assertEqual((acetime_t) 86400, dt.toEpochSeconds());
  assertEqual(LocalDate::kSunday, dt.dayOfWeek());

  // 2000-02-29 00:00:00Z Tuesday
  dt = ZonedDateTime::forComponents(2000, 2, 29, 0, 0, 0, TimeZone());
  assertEqual((acetime_t) 59, dt.toEpochDays());
  assertEqual((acetime_t) 11016, dt.toUnixDays());
  assertEqual((acetime_t) 86400 * 59, dt.toEpochSeconds());
  assertEqual(LocalDate::kTuesday, dt.dayOfWeek());

  // 2018-01-01 00:00:00Z Monday
  dt = ZonedDateTime::forComponents(2018, 1, 1, 0, 0, 0, TimeZone());
  assertEqual((acetime_t) 6575, dt.toEpochDays());
  assertEqual((acetime_t) 17532, dt.toUnixDays());
  assertEqual(6575 * (acetime_t) 86400, dt.toEpochSeconds());
  assertEqual(LocalDate::kMonday, dt.dayOfWeek());

  // 2018-01-01 00:00:00+00:15 Monday
  dt = ZonedDateTime::forComponents(2018, 1, 1, 0, 0, 0,
      TimeZone::forTimeOffset(TimeOffset::forHourMinute(0, 15)));
  assertEqual((acetime_t) 6574, dt.toEpochDays());
  assertEqual((acetime_t) 17531, dt.toUnixDays());
  assertEqual(6575 * (acetime_t) 86400 - 15*60, dt.toEpochSeconds());
  assertEqual(LocalDate::kMonday, dt.dayOfWeek());

  // 2038-01-19 03:14:07Z (largest value using Unix Epoch)
  dt = ZonedDateTime::forComponents(2038, 1, 19, 3, 14, 7, TimeZone());
  assertEqual((acetime_t) 13898, dt.toEpochDays());
  assertEqual((acetime_t) 24855, dt.toUnixDays());
  assertEqual((acetime_t) 1200798847, dt.toEpochSeconds());
  assertEqual(LocalDate::kTuesday, dt.dayOfWeek());

  // 2068-01-19 03:14:06Z (largest value for AceTime Epoch).
  dt = ZonedDateTime::forComponents(2068, 1, 19, 3, 14, 7, TimeZone());
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
  dt = ZonedDateTime::forComponents(1931, 12, 13, 20, 45, 53, TimeZone());
  assertEqual((acetime_t) -1200798847, dt.toUnixSeconds());
  udt = ZonedDateTime::forUnixSeconds(dt.toUnixSeconds(), TimeZone());
  assertTrue(dt == udt);

  // 1970-01-01 00:00:00Z
  dt = ZonedDateTime::forComponents(1970, 1, 1, 0, 0, 0, TimeZone());
  assertEqual((acetime_t) 0, dt.toUnixSeconds());
  udt = ZonedDateTime::forUnixSeconds(dt.toUnixSeconds(), TimeZone());
  assertTrue(dt == udt);

  // 2000-01-01 00:00:00Z
  dt = ZonedDateTime::forComponents(2000, 1, 1, 0, 0, 0, TimeZone());
  assertEqual((acetime_t) 946684800, dt.toUnixSeconds());
  udt = ZonedDateTime::forUnixSeconds(dt.toUnixSeconds(), TimeZone());
  assertTrue(dt == udt);


  // 2018-01-01 00:00:00Z
  dt = ZonedDateTime::forComponents(2018, 1, 1, 0, 0, 0, TimeZone());
  assertEqual((acetime_t) 1514764800, dt.toUnixSeconds());
  udt = ZonedDateTime::forUnixSeconds(dt.toUnixSeconds(), TimeZone());
  assertTrue(dt == udt);

  // 2018-08-30T06:45:01-07:00
  TimeZone tz = TimeZone::forTimeOffset(TimeOffset::forHours(-7));
  dt = ZonedDateTime::forComponents(2018, 8, 30, 6, 45, 1, tz);
  assertEqual((acetime_t) 1535636701, dt.toUnixSeconds());
  udt = ZonedDateTime::forUnixSeconds(dt.toUnixSeconds(), tz);
  assertTrue(dt == udt);

  // 2038-01-19 03:14:06Z (largest value - 1 using Unix Epoch)
  dt = ZonedDateTime::forComponents(2038, 1, 19, 3, 14, 6, TimeZone());
  assertEqual((acetime_t) (INT32_MAX - 1), dt.toUnixSeconds());
  udt = ZonedDateTime::forUnixSeconds(dt.toUnixSeconds(), TimeZone());
  assertTrue(dt == udt);
}

test(ZonedDateTimeTest_Manual, convertToTimeZone) {
  TimeZone stdTz = TimeZone::forTimeOffset(TimeOffset::forHours(-8));
  ZonedDateTime std = ZonedDateTime::forComponents(
      2018, 3, 11, 1, 59, 59, stdTz);
  acetime_t stdEpochSeconds = std.toEpochSeconds();

  TimeZone dstTz = stdTz;
  dstTz.setDstOffset(TimeOffset::forHours(1));
  ZonedDateTime dst = std.convertToTimeZone(dstTz);
  acetime_t dstEpochSeconds = dst.toEpochSeconds();

  assertEqual(stdEpochSeconds, dstEpochSeconds);

  assertEqual((int16_t) 2018, dst.year());
  assertEqual(3, dst.month());
  assertEqual(11, dst.day());
  assertEqual(2, dst.hour());
  assertEqual(59, dst.minute());
  assertEqual(59, dst.second());
  assertEqual(-7*60, dst.timeZone().getUtcOffset(stdEpochSeconds).toMinutes());
}

test(ZonedDateTimeTest_Manual, error) {
  TimeZone stdTz = TimeZone::forTimeOffset(TimeOffset::forHours(-8));

  ZonedDateTime zdt = ZonedDateTime::forEpochSeconds(
      LocalTime::kInvalidSeconds, stdTz);
  assertTrue(zdt.isError());

  zdt = ZonedDateTime::forUnixSeconds(LocalTime::kInvalidSeconds, stdTz);
  assertTrue(zdt.isError());
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
// data_time_mutation
// --------------------------------------------------------------------------

test(DateTimeMutationTest, increment) {
  ZonedDateTime dt = ZonedDateTime::forComponents(2001, 2, 3, 4, 5, 6,
      TimeZone());
  assertEqual((int16_t) 2001, dt.year());
  assertEqual(2, dt.month());
  assertEqual(3, dt.day());
  assertEqual(4, dt.hour());
  assertEqual(5, dt.minute());
  assertEqual(6, dt.second());
  assertEqual(0, dt.timeZone().getUtcOffset(0).toMinutes());

  zoned_date_time_mutation::incrementYear(dt);
  assertEqual((int16_t) 2002, dt.year());
  assertEqual(2, dt.month());
  assertEqual(3, dt.day());
  assertEqual(4, dt.hour());
  assertEqual(5, dt.minute());
  assertEqual(6, dt.second());
  assertEqual(0, dt.timeZone().getUtcOffset(0).toMinutes());

  zoned_date_time_mutation::incrementMonth(dt);
  assertEqual((int16_t) 2002, dt.year());
  assertEqual(3, dt.month());
  assertEqual(3, dt.day());
  assertEqual(4, dt.hour());
  assertEqual(5, dt.minute());
  assertEqual(6, dt.second());
  assertEqual(0, dt.timeZone().getUtcOffset(0).toMinutes());

  zoned_date_time_mutation::incrementDay(dt);
  assertEqual((int16_t) 2002, dt.year());
  assertEqual(3, dt.month());
  assertEqual(4, dt.day());
  assertEqual(4, dt.hour());
  assertEqual(5, dt.minute());
  assertEqual(6, dt.second());
  assertEqual(0, dt.timeZone().getUtcOffset(0).toMinutes());

  zoned_date_time_mutation::incrementHour(dt);
  assertEqual((int16_t) 2002, dt.year());
  assertEqual(3, dt.month());
  assertEqual(4, dt.day());
  assertEqual(5, dt.hour());
  assertEqual(5, dt.minute());
  assertEqual(6, dt.second());
  assertEqual(0, dt.timeZone().getUtcOffset(0).toMinutes());

  zoned_date_time_mutation::incrementMinute(dt);
  assertEqual((int16_t) 2002, dt.year());
  assertEqual(3, dt.month());
  assertEqual(4, dt.day());
  assertEqual(5, dt.hour());
  assertEqual(6, dt.minute());
  assertEqual(6, dt.second());
  assertEqual(0, dt.timeZone().getUtcOffset(0).toMinutes());
}

// --------------------------------------------------------------------------

void setup() {
#if defined(ARDUINO)
  delay(1000); // wait for stability to prevent garbage on SERIAL_PORT_MONITOR
#endif
  SERIAL_PORT_MONITOR.begin(115200);
  while(!SERIAL_PORT_MONITOR); // for the Arduino Leonardo/Micro only
}

void loop() {
  TestRunner::run();
}
