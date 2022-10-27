#line 2 "ZonedDateTimeTest.ino"

#include <AUnit.h>
#include <AceCommon.h>
#include <AceTime.h>

using namespace ace_time;

//---------------------------------------------------------------------------
// ZonedDateTime + Manual TimeZone
//---------------------------------------------------------------------------

test(ZonedDateTimeTest, accessors_mutators) {
  // accessors
  TimeZone tz = TimeZone::forHours(-8);
  ZonedDateTime dt = ZonedDateTime::forComponents(2001, 2, 3, 4, 5, 6, tz);
  assertEqual((int16_t) 2001, dt.year());
  assertEqual(2, dt.month());
  assertEqual(3, dt.day());
  assertEqual(4, dt.hour());
  assertEqual(5, dt.minute());
  assertEqual(6, dt.second());
  assertEqual(-8 * 60, dt.timeOffset().toMinutes());
  assertEqual(0, dt.fold());

  // mutators
  tz = TimeZone::forHours(-7);
  dt.year(2011);
  dt.month(12);
  dt.day(13);
  dt.hour(14);
  dt.minute(15);
  dt.second(16);
  dt.timeZone(tz);
  dt.fold(1);
  dt.normalize(); // must be called after timeZone() mutation
  assertEqual(2011, dt.year());
  assertEqual(12, dt.month());
  assertEqual(13, dt.day());
  assertEqual(14, dt.hour());
  assertEqual(15, dt.minute());
  assertEqual(16, dt.second());
  assertEqual(-7 * 60, dt.timeOffset().toMinutes());
  assertEqual(1, dt.fold());
}

test(ZonedDateTimeTest, constructor_with_fold) {
  TimeZone tz = TimeZone::forHours(-8);
  ZonedDateTime dt = ZonedDateTime::forComponents(
      2001, 2, 3, 4, 5, 6, tz, 1 /*fold*/);
  assertEqual((int16_t) 2001, dt.year());
  assertEqual(2, dt.month());
  assertEqual(3, dt.day());
  assertEqual(4, dt.hour());
  assertEqual(5, dt.minute());
  assertEqual(6, dt.second());
  assertEqual(-8 * 60, dt.timeOffset().toMinutes());
  assertEqual(1, dt.fold());
}

// Check that ZonedDateTime with Manual TimeZone agrees with simpler
// OffsetDateTime.
test(ZonedDateTimeTest_Manual, agreesWithOffsetDateTime) {
  TimeZone tz = TimeZone::forHours(-8);
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
  assertEqual((int32_t) -24856, dt.toEpochDays());
  assertEqual((int32_t) -13899, dt.toUnixDays());
  assertEqual((acetime_t) (INT32_MIN + 1), dt.toEpochSeconds());
  assertEqual(LocalDate::kSunday, dt.dayOfWeek());

  // 2000-01-01 00:00:00Z Saturday
  dt = ZonedDateTime::forComponents(2000, 1, 1, 0, 0, 0, TimeZone());
  assertEqual((int32_t) 0, dt.toEpochDays());
  assertEqual((int32_t) 10957, dt.toUnixDays());
  assertEqual((acetime_t) 0, dt.toEpochSeconds());
  assertEqual(LocalDate::kSaturday, dt.dayOfWeek());

  // 2000-01-02 00:00:00Z Sunday
  dt = ZonedDateTime::forComponents(2000, 1, 2, 0, 0, 0, TimeZone());
  assertEqual((int32_t) 1, dt.toEpochDays());
  assertEqual((int32_t) 10958, dt.toUnixDays());
  assertEqual((acetime_t) 86400, dt.toEpochSeconds());
  assertEqual(LocalDate::kSunday, dt.dayOfWeek());

  // 2000-02-29 00:00:00Z Tuesday
  dt = ZonedDateTime::forComponents(2000, 2, 29, 0, 0, 0, TimeZone());
  assertEqual((int32_t) 59, dt.toEpochDays());
  assertEqual((int32_t) 11016, dt.toUnixDays());
  assertEqual((acetime_t) 86400 * 59, dt.toEpochSeconds());
  assertEqual(LocalDate::kTuesday, dt.dayOfWeek());

  // 2018-01-01 00:00:00Z Monday
  dt = ZonedDateTime::forComponents(2018, 1, 1, 0, 0, 0, TimeZone());
  assertEqual((int32_t) 6575, dt.toEpochDays());
  assertEqual((int32_t) 17532, dt.toUnixDays());
  assertEqual(6575 * (acetime_t) 86400, dt.toEpochSeconds());
  assertEqual(LocalDate::kMonday, dt.dayOfWeek());

  // 2018-01-01 00:00:00+00:15 Monday
  dt = ZonedDateTime::forComponents(2018, 1, 1, 0, 0, 0,
      TimeZone::forHourMinute(0, 15));
  assertEqual((int32_t) 6574, dt.toEpochDays());
  assertEqual((int32_t) 17531, dt.toUnixDays());
  assertEqual(6575 * (acetime_t) 86400 - 15*60, dt.toEpochSeconds());
  assertEqual(LocalDate::kMonday, dt.dayOfWeek());

  // 2038-01-19 03:14:07Z (largest value using Unix Epoch)
  dt = ZonedDateTime::forComponents(2038, 1, 19, 3, 14, 7, TimeZone());
  assertEqual((int32_t) 13898, dt.toEpochDays());
  assertEqual((int32_t) 24855, dt.toUnixDays());
  assertEqual((acetime_t) 1200798847, dt.toEpochSeconds());
  assertEqual(LocalDate::kTuesday, dt.dayOfWeek());

  // 2068-01-19 03:14:06Z (largest value for AceTime Epoch).
  dt = ZonedDateTime::forComponents(2068, 1, 19, 3, 14, 7, TimeZone());
  assertEqual((int32_t) 24855, dt.toEpochDays());
  assertEqual((int32_t) 35812, dt.toUnixDays());
  assertEqual((acetime_t) (INT32_MAX), dt.toEpochSeconds());
  assertEqual(LocalDate::kThursday, dt.dayOfWeek());
}

test(ZonedDateTimeTest_Manual, toAndForUnixSeconds64) {
  ZonedDateTime dt;
  ZonedDateTime udt;

  // 1931-12-13 20:45:52Z, smalltest datetime using int32_t from AceTime Epoch.
  // Let's use +1 of that since INT_MIN will be used to indicate an error.
  dt = ZonedDateTime::forComponents(1931, 12, 13, 20, 45, 53, TimeZone());
  assertEqual((int64_t) -1200798847, dt.toUnixSeconds64());
  udt = ZonedDateTime::forUnixSeconds64(dt.toUnixSeconds64(), TimeZone());
  assertTrue(dt == udt);

  // 1970-01-01 00:00:00Z
  dt = ZonedDateTime::forComponents(1970, 1, 1, 0, 0, 0, TimeZone());
  assertEqual((int64_t) 0, dt.toUnixSeconds64());
  udt = ZonedDateTime::forUnixSeconds64(dt.toUnixSeconds64(), TimeZone());
  assertTrue(dt == udt);

  // 2000-01-01 00:00:00Z
  dt = ZonedDateTime::forComponents(2000, 1, 1, 0, 0, 0, TimeZone());
  assertEqual((int64_t) 946684800, dt.toUnixSeconds64());
  udt = ZonedDateTime::forUnixSeconds64(dt.toUnixSeconds64(), TimeZone());
  assertTrue(dt == udt);


  // 2018-01-01 00:00:00Z
  dt = ZonedDateTime::forComponents(2018, 1, 1, 0, 0, 0, TimeZone());
  assertEqual((int64_t) 1514764800, dt.toUnixSeconds64());
  udt = ZonedDateTime::forUnixSeconds64(dt.toUnixSeconds64(), TimeZone());
  assertTrue(dt == udt);

  // 2018-08-30T06:45:01-07:00
  TimeZone tz = TimeZone::forHours(-7);
  dt = ZonedDateTime::forComponents(2018, 8, 30, 6, 45, 1, tz);
  assertEqual((int64_t) 1535636701, dt.toUnixSeconds64());
  udt = ZonedDateTime::forUnixSeconds64(dt.toUnixSeconds64(), tz);
  assertTrue(dt == udt);

  // 2038-01-19 03:14:06Z (largest value - 1 using Unix Epoch)
  dt = ZonedDateTime::forComponents(2038, 1, 19, 3, 14, 6, TimeZone());
  assertEqual((int64_t) (INT32_MAX - 1), dt.toUnixSeconds64());
  udt = ZonedDateTime::forUnixSeconds64(dt.toUnixSeconds64(), TimeZone());
  assertTrue(dt == udt);
}

test(ZonedDateTimeTest_Manual, toAndForUnixSeconds64_extended) {
  ZonedDateTime dt;
  ZonedDateTime udt;

  // 2038-01-19 03:14:08Z (largest value + 1 using Unix Epoch)
  dt = ZonedDateTime::forComponents(2038, 1, 19, 3, 14, 8, TimeZone());
  assertEqual((int64_t) INT32_MAX + 1, dt.toUnixSeconds64());
  udt = ZonedDateTime::forUnixSeconds64(dt.toUnixSeconds64(), TimeZone());
  assertTrue(dt == udt);

  // 2068-01-19 03:14:07Z (largest value for 32-bit AceTime seconds) should work
  // with 64-bit Unix seconds.
  dt = ZonedDateTime::forComponents(2068, 1, 19, 3, 14, 7, TimeZone());
  assertEqual((int64_t) 3094168447, dt.toUnixSeconds64());
  udt = ZonedDateTime::forUnixSeconds64(dt.toUnixSeconds64(), TimeZone());
  assertTrue(dt == udt);

  // One second after that, forUnixSeconds64() should fail because we cannot
  // represent this datetime using 32-bit AceTime seconds internally.
  dt = ZonedDateTime::forUnixSeconds64((int64_t) 3094168447 + 1, TimeZone());
  assertTrue(dt.isError());

  // Verify error sentinel.
  dt = ZonedDateTime::forUnixSeconds64(
      LocalDate::kInvalidUnixSeconds64, TimeZone());
  assertTrue(dt.isError());
}

test(ZonedDateTimeTest_Manual, convertToTimeZone) {
  TimeZone stdTz = TimeZone::forHours(-8);
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
  TimeZone stdTz = TimeZone::forHours(-8);

  ZonedDateTime zdt = ZonedDateTime::forEpochSeconds(
      LocalDate::kInvalidEpochSeconds, stdTz);
  assertTrue(zdt.isError());

  zdt = ZonedDateTime::forUnixSeconds64(
      LocalDate::kInvalidUnixSeconds64, stdTz);
  assertTrue(zdt.isError());
}

//---------------------------------------------------------------------------

test(ZonedDateTimeTest, spotcheck_epoch2000) {
  // Change current epoch year to 2000, so the epoch is 2000-01-01T00:00:00.
  int16_t savedEpochYear = LocalDate::currentEpochYear();
  LocalDate::currentEpochYear(2000);

  auto minDt = ZonedDateTime::forEpochSeconds(LocalDate::kMinEpochSeconds,
      TimeZone());
  auto expected = ZonedDateTime::forComponents(1931, 12, 13, 20, 45, 53,
      TimeZone());
  assertTrue(expected == minDt);

  auto maxDt = ZonedDateTime::forEpochSeconds(LocalDate::kMaxEpochSeconds,
      TimeZone());
  expected = ZonedDateTime::forComponents(2068, 1, 19, 3, 14, 7, TimeZone());
  assertTrue(expected == maxDt);

  // Verify that toUnixDays() does not change if currentEpochYear() is changed.
  auto dt = ZonedDateTime::forComponents(1931, 12, 13, 20, 45, 53,
      TimeZone());
  assertEqual((int32_t) -13899, dt.toUnixDays());
  dt = ZonedDateTime::forComponents(2000, 1, 1, 0, 0, 0, TimeZone());
  assertEqual((int32_t) 10957, dt.toUnixDays());
  dt = ZonedDateTime::forComponents(2038, 1, 19, 3, 14, 7, TimeZone());
  assertEqual((int32_t) 24855, dt.toUnixDays());

  // Reset to the previous current epoch year.
  LocalDate::currentEpochYear(savedEpochYear);
}

test(ZonedDateTimeTest, spotcheck_epoch2050) {
  // Change current epoch year to 2050, so the epoch is 2050-01-01T00:00:00.
  int16_t savedEpochYear = LocalDate::currentEpochYear();
  LocalDate::currentEpochYear(2050);

  // Same min date as epoch 2000, but 50 years later.
  auto minDt = ZonedDateTime::forEpochSeconds(LocalDate::kMinEpochSeconds,
      TimeZone());
  auto expected = ZonedDateTime::forComponents(1981, 12, 13, 20, 45, 53,
      TimeZone());
  assertTrue(expected == minDt);

  // Almost the same max date as epoch 2000, but one day later on Jan 20 instead
  // of the Jan 19, because 2000 was a leap year, but 2100 is not.
  auto maxDt = ZonedDateTime::forEpochSeconds(LocalDate::kMaxEpochSeconds,
      TimeZone());
  expected = ZonedDateTime::forComponents(2118, 1, 20, 3, 14, 7, TimeZone());
  assertTrue(expected == maxDt);

  // Verify that toUnixDays() does not change if currentEpochYear() is changed.
  auto dt = ZonedDateTime::forComponents(1931, 12, 13, 20, 45, 53, TimeZone());
  assertEqual((int32_t) -13899, dt.toUnixDays());
  dt = ZonedDateTime::forComponents(2000, 1, 1, 0, 0, 0, TimeZone());
  assertEqual((int32_t) 10957, dt.toUnixDays());
  dt = ZonedDateTime::forComponents(2038, 1, 19, 3, 14, 7, TimeZone());
  assertEqual((int32_t) 24855, dt.toUnixDays());

  // Reset to the previous current epoch year.
  LocalDate::currentEpochYear(savedEpochYear);
}

test(ZonedDateTimeTest, spotcheck_epoch2100) {
  // Change current epoch year to 2100, so the epoch is 2100-01-01T00:00:00.
  int16_t savedEpochYear = LocalDate::currentEpochYear();
  LocalDate::currentEpochYear(2100);

  // Same min date as epoch 2000, but 100 years later.
  auto minDt = ZonedDateTime::forEpochSeconds(LocalDate::kMinEpochSeconds,
      TimeZone());
  auto expected = ZonedDateTime::forComponents(2031, 12, 13, 20, 45, 53,
      TimeZone());
  assertTrue(expected == minDt);

  // Almost the same max date as epoch 2000, but one day later on Jan 20 instead
  // of the Jan 19, because 2000 was a leap year, but 2100 is not.
  auto maxDt = ZonedDateTime::forEpochSeconds(LocalDate::kMaxEpochSeconds,
      TimeZone());
  expected = ZonedDateTime::forComponents(2168, 1, 20, 3, 14, 7, TimeZone());
  assertTrue(expected == maxDt);

  // Verify that toUnixDays() does not change if currentEpochYear() is changed.
  auto dt = ZonedDateTime::forComponents(1931, 12, 13, 20, 45, 53, TimeZone());
  assertEqual((int32_t) -13899, dt.toUnixDays());
  dt = ZonedDateTime::forComponents(2000, 1, 1, 0, 0, 0, TimeZone());
  assertEqual((int32_t) 10957, dt.toUnixDays());
  dt = ZonedDateTime::forComponents(2038, 1, 19, 3, 14, 7, TimeZone());
  assertEqual((int32_t) 24855, dt.toUnixDays());

  // Reset to the previous current epoch year.
  LocalDate::currentEpochYear(savedEpochYear);
}

//---------------------------------------------------------------------------
// forDateString()
//---------------------------------------------------------------------------

test(ZonedDateTimeTest, forDateString) {
  // exact ISO8601 format
  auto dt = ZonedDateTime::forDateString(F("2018-08-31T13:48:01-07:00"));
  assertFalse(dt.isError());
  assertEqual((int16_t) 2018, dt.year());
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

//---------------------------------------------------------------------------
// printTo()
//---------------------------------------------------------------------------

test(ZonedDateTimeTest_Manual, printTo) {
  ace_common::PrintStr<64> dateString;

  ZonedDateTime dt = ZonedDateTime::forComponents(2001, 2, 3, 4, 5, 6,
      TimeZone());
  dt.printTo(dateString);
  assertEqual(dateString.cstr(), "2001-02-03T04:05:06+00:00[UTC]");

  dateString.flush();
  TimeZone stdTz = TimeZone::forHours(-8);
  ZonedDateTime std = ZonedDateTime::forComponents(
      2018, 3, 11, 1, 59, 59, stdTz);
  std.printTo(dateString);
  assertEqual(dateString.cstr(), "2018-03-11T01:59:59-08:00[-08:00+00:00]");
}

//---------------------------------------------------------------------------
// data_time_mutation
//---------------------------------------------------------------------------

test(ZonedDateTimeMutationTest, increment) {
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

//---------------------------------------------------------------------------

void setup() {
#if ! defined(EPOXY_DUINO)
  delay(1000); // wait to prevent garbage on SERIAL_PORT_MONITOR
#endif
  SERIAL_PORT_MONITOR.begin(115200);
  while (!SERIAL_PORT_MONITOR); // Leonardo/Micro
}

void loop() {
  aunit::TestRunner::run();
}
