#line 2 "LocalDateTimeTest.ino"

#include <AUnit.h>
#include <AceCommon.h>
#include <AceTime.h>
#include <ace_time/testing/EpochYearContext.h>

using ace_common::PrintStr;
using namespace ace_time;

//---------------------------------------------------------------------------
// LocalDateTime
//---------------------------------------------------------------------------

test(LocalDateTimeTest, accessors_mutators) {
  // accessors
  LocalDateTime dt = LocalDateTime::forComponents(2001, 2, 3, 4, 5, 6);
  assertEqual((int16_t) 2001, dt.year());
  assertEqual(2, dt.month());
  assertEqual(3, dt.day());
  assertEqual(4, dt.hour());
  assertEqual(5, dt.minute());
  assertEqual(6, dt.second());
  assertEqual(0, dt.fold());

  // mutators
  dt.year(2011);
  dt.month(12);
  dt.day(13);
  dt.hour(14);
  dt.minute(15);
  dt.second(16);
  dt.fold(1);
  assertEqual(2011, dt.year());
  assertEqual(12, dt.month());
  assertEqual(13, dt.day());
  assertEqual(14, dt.hour());
  assertEqual(15, dt.minute());
  assertEqual(16, dt.second());
  assertEqual(1, dt.fold());
}

test(LocalDateTimeTest, constructor_with_fold) {
  LocalDateTime dt = LocalDateTime::forComponents(
      2001, 2, 3, 4, 5, 6, 1 /*fold*/);
  assertEqual((int16_t) 2001, dt.year());
  assertEqual(2, dt.month());
  assertEqual(3, dt.day());
  assertEqual(4, dt.hour());
  assertEqual(5, dt.minute());
  assertEqual(6, dt.second());
  assertEqual(1, dt.fold());
}

test(LocalDateTimeTest, invalidSeconds) {
  LocalDateTime dt = LocalDateTime::forEpochSeconds(
      LocalDate::kInvalidEpochSeconds);
  assertTrue(dt.isError());
  assertEqual(LocalDate::kInvalidEpochSeconds, dt.toEpochSeconds());
  assertEqual(LocalDate::kInvalidEpochDays, dt.toEpochDays());
}

test(LocalDateTimeTest, forError) {
  LocalDateTime dt = LocalDateTime::forError();
  assertTrue(dt.isError());
}

test(LocalDateTimeTest, isError) {
  // Good LocalDateTime.
  // 2018-01-01 00:00:00Z
  LocalDateTime dt = LocalDateTime::forComponents(2018, 1, 1, 0, 0, 0);
  assertFalse(dt.isError());

  // bad year
  dt = LocalDateTime::forComponents(-1, 1, 1, 0, 0, 0);
  assertTrue(dt.isError());

  // allowed, min FROM field
  dt = LocalDateTime::forComponents(0, 1, 1, 0, 0, 0);
  assertFalse(dt.isError());

  // allowed, max TO field
  dt = LocalDateTime::forComponents(9999, 1, 1, 0, 0, 0);
  assertFalse(dt.isError());

  // allowed, max UNTIL field
  dt = LocalDateTime::forComponents(10000, 1, 1, 0, 0, 0);
  assertFalse(dt.isError());

  // bad month
  dt = LocalDateTime::forComponents(2018, 0, 1, 0, 0, 0);
  assertTrue(dt.isError());

  // bad month
  dt = LocalDateTime::forComponents(2018, 13, 0, 0, 0, 0);
  assertTrue(dt.isError());

  // bad day
  dt = LocalDateTime::forComponents(2018, 1, 0, 0, 0, 0);
  assertTrue(dt.isError());

  // bad day
  dt = LocalDateTime::forComponents(2018, 1, 32, 0, 0, 0);
  assertTrue(dt.isError());

  // bad hour
  dt = LocalDateTime::forComponents(2018, 1, 1, 25, 0, 0);
  assertTrue(dt.isError());

  // bad minute
  dt = LocalDateTime::forComponents(2018, 1, 1, 0, 60, 0);
  assertTrue(dt.isError());

  // bad second
  dt = LocalDateTime::forComponents(2018, 1, 1, 0, 0, 60);
  assertTrue(dt.isError());
}

test(LocalDateTimeTest, forComponents) {
  testing::EpochYearContext context(2000);
  LocalDateTime dt;

  // 1931-12-13 20:45:52Z, smalltest datetime using int32_t from AceTime Epoch.
  // Let's use +1 of that since INT_MIN will be used to indicate an error.
  dt = LocalDateTime::forComponents(1931, 12, 13, 20, 45, 53);
  assertEqual((int32_t) -24856, dt.toEpochDays());
  assertEqual((int32_t) -13899, dt.toUnixDays());
  assertEqual(LocalDate::kMinEpochSeconds, dt.toEpochSeconds());
  assertEqual(LocalDate::kSunday, dt.dayOfWeek());

  // 2000-01-01 00:00:00Z Saturday
  dt = LocalDateTime::forComponents(2000, 1, 1, 0, 0, 0);
  assertEqual((int32_t) 0, dt.toEpochDays());
  assertEqual((int32_t) 10957, dt.toUnixDays());
  assertEqual((acetime_t) 0, dt.toEpochSeconds());
  assertEqual(LocalDate::kSaturday, dt.dayOfWeek());

  // 2000-01-02 00:00:00Z Sunday
  dt = LocalDateTime::forComponents(2000, 1, 2, 0, 0, 0);
  assertEqual((int32_t) 1, dt.toEpochDays());
  assertEqual((int32_t) 10958, dt.toUnixDays());
  assertEqual((acetime_t) 86400, dt.toEpochSeconds());
  assertEqual(LocalDate::kSunday, dt.dayOfWeek());

  // 2000-02-29 00:00:00Z Tuesday
  dt = LocalDateTime::forComponents(2000, 2, 29, 0, 0, 0);
  assertEqual((int32_t) 59, dt.toEpochDays());
  assertEqual((int32_t) 11016, dt.toUnixDays());
  assertEqual((acetime_t) 86400 * 59, dt.toEpochSeconds());
  assertEqual(LocalDate::kTuesday, dt.dayOfWeek());

  // 2018-01-01 00:00:00Z Monday
  dt = LocalDateTime::forComponents(2018, 1, 1, 0, 0, 0);
  assertEqual((int32_t) 6575, dt.toEpochDays());
  assertEqual((int32_t) 17532, dt.toUnixDays());
  assertEqual(6575 * (acetime_t) 86400, dt.toEpochSeconds());
  assertEqual(LocalDate::kMonday, dt.dayOfWeek());

  // 2038-01-19 03:14:07Z (largest value using Unix Epoch)
  dt = LocalDateTime::forComponents(2038, 1, 19, 3, 14, 7);
  assertEqual((int32_t) 13898, dt.toEpochDays());
  assertEqual((int32_t) 24855, dt.toUnixDays());
  assertEqual((acetime_t) 1200798847, dt.toEpochSeconds());
  assertEqual(LocalDate::kTuesday, dt.dayOfWeek());

  // 2068-01-19 03:14:06Z (one second before largest AceTime Epoch)
  dt = LocalDateTime::forComponents(2068, 1, 19, 3, 14, 6);
  assertEqual((int32_t) 24855, dt.toEpochDays());
  assertEqual((int32_t) 35812, dt.toUnixDays());
  assertEqual((acetime_t) (INT32_MAX - 1), dt.toEpochSeconds());
  assertEqual(LocalDate::kThursday, dt.dayOfWeek());

  // 2068-01-19 03:14:07Z (largest value for AceTime Epoch).
  dt = LocalDateTime::forComponents(2068, 1, 19, 3, 14, 7);
  assertEqual((int32_t) 24855, dt.toEpochDays());
  assertEqual((int32_t) 35812, dt.toUnixDays());
  assertEqual((acetime_t) INT32_MAX, dt.toEpochSeconds());
  assertEqual(LocalDate::kThursday, dt.dayOfWeek());
}

test(LocalDateTimeTest, toAndForUnixSeconds64) {
  LocalDateTime dt;
  LocalDateTime udt;

  // Verify error sentinel.
  dt = LocalDateTime::forUnixSeconds64(LocalDate::kInvalidUnixSeconds64);
  assertTrue(dt.isError());

  // Verify that 64-bit unixSeconds allows dates beyond 32-bit limit.
  // 1970 - 1770 = 200 years
  //      = 200 * 365 + (200/4) leap years - 2 (1800, 1900 are not leap)
  //      = 73048 days
  dt = LocalDateTime::forComponents(1770, 1, 1, 0, 0, 0);
  assertEqual((int64_t) -73048 * 86400, dt.toUnixSeconds64());
  udt = LocalDateTime::forUnixSeconds64((int64_t) -73048 * 86400);
  assertTrue(dt == udt);

  // 1931-12-13 20:45:52Z, smalltest datetime using int32_t from AceTime Epoch.
  // Let's use +1 of that since INT_MIN will be used to indicate an error.
  dt = LocalDateTime::forComponents(1931, 12, 13, 20, 45, 53);
  assertEqual((int64_t) -1200798847, dt.toUnixSeconds64());
  udt = LocalDateTime::forUnixSeconds64(dt.toUnixSeconds64());
  assertTrue(dt == udt);

  // 1970-01-01 00:00:00Z
  dt = LocalDateTime::forComponents(1970, 1, 1, 0, 0, 0);
  assertEqual((int64_t) 0, dt.toUnixSeconds64());
  udt = LocalDateTime::forUnixSeconds64(dt.toUnixSeconds64());
  assertTrue(dt == udt);

  // 2000-01-01 00:00:00Z
  dt = LocalDateTime::forComponents(2000, 1, 1, 0, 0, 0);
  assertEqual((int64_t) 946684800, dt.toUnixSeconds64());
  udt = LocalDateTime::forUnixSeconds64(dt.toUnixSeconds64());
  assertTrue(dt == udt);

  // 2018-01-01 00:00:00Z
  dt = LocalDateTime::forComponents(2018, 1, 1, 0, 0, 0);
  assertEqual((int64_t) 1514764800, dt.toUnixSeconds64());
  udt = LocalDateTime::forUnixSeconds64(dt.toUnixSeconds64());
  assertTrue(dt == udt);

  // 2038-01-19 03:14:06Z (largest value - 1 using 32-bit Unix Seconds)
  dt = LocalDateTime::forComponents(2038, 1, 19, 3, 14, 6);
  assertEqual((int64_t) (INT32_MAX - 1), dt.toUnixSeconds64());
  udt = LocalDateTime::forUnixSeconds64(dt.toUnixSeconds64());
  assertTrue(dt == udt);

  // 2038-01-19 03:14:08Z (largest value + 1 using 32-bit Unix Seconds)
  dt = LocalDateTime::forComponents(2038, 1, 19, 3, 14, 8);
  assertEqual((int64_t) INT32_MAX + 1, dt.toUnixSeconds64());
  udt = LocalDateTime::forUnixSeconds64(dt.toUnixSeconds64());
  assertTrue(dt == udt);

  // 2068-01-19 03:14:07Z (largest value for 32-bit AceTime seconds) should work
  // with 64-bit Unix seconds.
  dt = LocalDateTime::forComponents(2068, 1, 19, 3, 14, 7);
  assertEqual((int64_t) 3094168447, dt.toUnixSeconds64());
  udt = LocalDateTime::forUnixSeconds64(dt.toUnixSeconds64());
  assertTrue(dt == udt);

  // Verify that year 2170 works just fine with 64-bit Unix Seconds.
  // 2170 - 1970 = 200 years
  //    = 73049 days, instead of 73048 days, because 2000 was a leap year.
  dt = LocalDateTime::forComponents(2170, 1, 1, 0, 0, 0);
  assertEqual((int64_t) 73049 * 86400, dt.toUnixSeconds64());
  udt = LocalDateTime::forUnixSeconds64((int64_t) 73049 * 86400);
  assertTrue(dt == udt);
}

test(LocalDateTimeTest, forEpochSeconds) {
  testing::EpochYearContext context(2000);

  // 2029-12-31 23:59:59Z Monday
  LocalDateTime dt = LocalDateTime::forEpochSeconds(
      10958 * (acetime_t) 86400 - 1);

  assertEqual((int16_t) 2029, dt.year());
  assertEqual(12, dt.month());
  assertEqual(31, dt.day());
  assertEqual(23, dt.hour());
  assertEqual(59, dt.minute());
  assertEqual(59, dt.second());
  assertEqual(LocalDate::kMonday, dt.dayOfWeek());
  assertEqual(0, dt.fold());
}

test(LocalDateTimeTest, forEpochSeconds_withFold) {
  testing::EpochYearContext context(2000);

  // 2029-12-31 23:59:59Z Monday
  LocalDateTime dt = LocalDateTime::forEpochSeconds(
      10958 * (acetime_t) 86400 - 1, 1 /*fold*/);

  assertEqual((int16_t) 2029, dt.year());
  assertEqual(12, dt.month());
  assertEqual(31, dt.day());
  assertEqual(23, dt.hour());
  assertEqual(59, dt.minute());
  assertEqual(59, dt.second());
  assertEqual(LocalDate::kMonday, dt.dayOfWeek());
  assertEqual(1, dt.fold());
}

test(LocalDateTimeTest, compareTo) {
  LocalDateTime a = LocalDateTime::forComponents(2018, 1, 1, 12, 0, 0);
  LocalDateTime b = LocalDateTime::forComponents(2018, 1, 1, 12, 0, 0);
  assertEqual(a.compareTo(b), 0);
  assertTrue(a == b);
  assertFalse(a != b);

  a = LocalDateTime::forComponents(2018, 1, 1, 12, 0, 0);
  b = LocalDateTime::forComponents(2018, 1, 1, 12, 0, 1);
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);

  a = LocalDateTime::forComponents(2018, 1, 1, 12, 0, 0);
  b = LocalDateTime::forComponents(2018, 1, 1, 12, 1, 0);
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);

  a = LocalDateTime::forComponents(2018, 1, 1, 12, 0, 0);
  b = LocalDateTime::forComponents(2018, 1, 2, 12, 0, 0);
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);

  a = LocalDateTime::forComponents(2018, 1, 1, 12, 0, 0);
  b = LocalDateTime::forComponents(2018, 2, 1, 12, 0, 0);
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);

  a = LocalDateTime::forComponents(2018, 1, 1, 12, 0, 0);
  b = LocalDateTime::forComponents(2019, 1, 1, 12, 0, 0);
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);
}

test(LocalDateTimeTest, dayOfWeek) {
  // 2018-01-01 00:00:00Z Monday
  LocalDateTime dt = LocalDateTime::forComponents(2018, 1, 1, 0, 0, 0);
  assertEqual(LocalDate::kMonday, dt.dayOfWeek());

  dt.hour(23); // 2018-01-01 23:00:00Z, no change to dayOfWeek
  assertEqual(LocalDate::kMonday, dt.dayOfWeek());

  dt.minute(40); // 2018-01-01 23:40:00Z, no change to dayOfWeek
  assertEqual(LocalDate::kMonday, dt.dayOfWeek());

  dt.second(3); // 2018-01-01 23:40:03Z, no change to dayOfWeek
  assertEqual(LocalDate::kMonday, dt.dayOfWeek());

  dt.day(2); // 2018-01-02 23:40:03+00:45, changes dayOfWeek
  assertEqual(LocalDate::kTuesday, dt.dayOfWeek());

  dt.month(2); // 2018-02-02 23:40:03+00:45, changes dayOfWeek
  assertEqual(LocalDate::kFriday, dt.dayOfWeek());

  dt.year(2020); // 2020-02-02 23:40:03+00:45, changes dayOfWeek
  assertEqual(LocalDate::kSunday, dt.dayOfWeek());
}

test(LocalDateTimeTest, forDateString) {
  // exact ISO8601 format
  LocalDateTime dt = LocalDateTime::forDateString(F("2018-08-31T13:48:01"));
  assertFalse(dt.isError());
  assertEqual((int16_t) 2018, dt.year());
  assertEqual(8, dt.month());
  assertEqual(31, dt.day());
  assertEqual(13, dt.hour());
  assertEqual(48, dt.minute());
  assertEqual(1, dt.second());
  assertEqual(LocalDate::kFriday, dt.dayOfWeek());

  // parser does not care about most separators, this may change in the future
  dt = LocalDateTime::forDateString(F("2018/08/31 13#48#01"));
  assertFalse(dt.isError());
  assertEqual((int16_t) 2018, dt.year());
  assertEqual(8, dt.month());
  assertEqual(31, dt.day());
  assertEqual(13, dt.hour());
  assertEqual(48, dt.minute());
  assertEqual(1, dt.second());
  assertEqual(LocalDate::kFriday, dt.dayOfWeek());
}

test(LocalDateTimeTest, forDateString_errors) {
  // empty string, too short
  LocalDateTime dt = LocalDateTime::forDateString("");
  assertTrue(dt.isError());

  // not enough components
  dt = LocalDateTime::forDateString(F("2018-08-31"));
  assertTrue(dt.isError());

  // too long
  dt = LocalDateTime::forDateString(F("2018-08-31T13:48:01-07:00"));
  assertTrue(dt.isError());

  // too short
  dt = LocalDateTime::forDateString(F("2018-08-31T13:48"));
  assertTrue(dt.isError());
}

test(LocalDateTimeTest, printTo) {
  LocalDateTime dt = LocalDateTime::forComponents(2020, 10, 30, 1, 2, 3);
  PrintStr<30> dateString;
  dt.printTo(dateString);
  assertEqual(dateString.cstr(), "2020-10-30T01:02:03");
}

//---------------------------------------------------------------------------

test(LocalDateTimeTest, spotcheck_epoch2000) {
  // Change current epoch year to 2000, so the epoch is 2000-01-01T00:00:00.
  testing::EpochYearContext context(2000);

  auto minDt = LocalDateTime::forEpochSeconds(LocalDate::kMinEpochSeconds);
  auto expected = LocalDateTime::forComponents(1931, 12, 13, 20, 45, 53);
  assertTrue(expected == minDt);

  auto maxDt = LocalDateTime::forEpochSeconds(LocalDate::kMaxEpochSeconds);
  expected = LocalDateTime::forComponents(2068, 1, 19, 3, 14, 7);
  assertTrue(expected == maxDt);

  // Verify that toUnixDays() does not change if currentEpochYear() is changed.
  auto dt = LocalDateTime::forComponents(1931, 12, 13, 20, 45, 53);
  assertEqual((int32_t) -13899, dt.toUnixDays());
  dt = LocalDateTime::forComponents(2000, 1, 1, 0, 0, 0);
  assertEqual((int32_t) 10957, dt.toUnixDays());
  dt = LocalDateTime::forComponents(2038, 1, 19, 3, 14, 7);
  assertEqual((int32_t) 24855, dt.toUnixDays());
}

test(LocalDateTimeTest, spotcheck_epoch2050) {
  // Change current epoch year to 2050, so the epoch is 2050-01-01T00:00:00.
  testing::EpochYearContext context(2050);

  // Same min date as epoch 2000, but 50 years later.
  auto minDt = LocalDateTime::forEpochSeconds(LocalDate::kMinEpochSeconds);
  auto expected = LocalDateTime::forComponents(1981, 12, 13, 20, 45, 53);
  assertTrue(expected == minDt);

  // Almost the same max date as epoch 2000, but one day later on Jan 20 instead
  // of the Jan 19, because 2000 was a leap year, but 2100 is not.
  auto maxDt = LocalDateTime::forEpochSeconds(LocalDate::kMaxEpochSeconds);
  expected = LocalDateTime::forComponents(2118, 1, 20, 3, 14, 7);
  assertTrue(expected == maxDt);

  // Verify that toUnixDays() does not change if currentEpochYear() is changed.
  auto dt = LocalDateTime::forComponents(1931, 12, 13, 20, 45, 53);
  assertEqual((int32_t) -13899, dt.toUnixDays());
  dt = LocalDateTime::forComponents(2000, 1, 1, 0, 0, 0);
  assertEqual((int32_t) 10957, dt.toUnixDays());
  dt = LocalDateTime::forComponents(2038, 1, 19, 3, 14, 7);
  assertEqual((int32_t) 24855, dt.toUnixDays());
}

test(LocalDateTimeTest, spotcheck_epoch2100) {
  // Change current epoch year to 2100, so the epoch is 2100-01-01T00:00:00.
  testing::EpochYearContext context(2100);

  // Same min date as epoch 2000, but 100 years later.
  auto minDt = LocalDateTime::forEpochSeconds(LocalDate::kMinEpochSeconds);
  auto expected = LocalDateTime::forComponents(2031, 12, 13, 20, 45, 53);
  assertTrue(expected == minDt);

  // Almost the same max date as epoch 2000, but one day later on Jan 20 instead
  // of the Jan 19, because 2000 was a leap year, but 2100 is not.
  auto maxDt = LocalDateTime::forEpochSeconds(LocalDate::kMaxEpochSeconds);
  expected = LocalDateTime::forComponents(2168, 1, 20, 3, 14, 7);
  assertTrue(expected == maxDt);

  // Verify that toUnixDays() does not change if currentEpochYear() is changed.
  auto dt = LocalDateTime::forComponents(1931, 12, 13, 20, 45, 53);
  assertEqual((int32_t) -13899, dt.toUnixDays());
  dt = LocalDateTime::forComponents(2000, 1, 1, 0, 0, 0);
  assertEqual((int32_t) 10957, dt.toUnixDays());
  dt = LocalDateTime::forComponents(2038, 1, 19, 3, 14, 7);
  assertEqual((int32_t) 24855, dt.toUnixDays());
}

//---------------------------------------------------------------------------

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
