#line 2 "LocalDateTimeTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time;
using namespace ace_time::common;

// --------------------------------------------------------------------------
// LocalDateTime
// --------------------------------------------------------------------------

test(LocalDateTimeTest, accessors) {
  LocalDateTime dt = LocalDateTime::forComponents(2001, 2, 3, 4, 5, 6);
  assertEqual((int16_t) 2001, dt.year());
  assertEqual(1, dt.yearTiny());
  assertEqual(2, dt.month());
  assertEqual(3, dt.day());
  assertEqual(4, dt.hour());
  assertEqual(5, dt.minute());
  assertEqual(6, dt.second());
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
  dt = LocalDateTime::forComponents(0, 1, 1, 0, 0, 0);
  assertTrue(dt.isError());

  // bad year
  dt = LocalDateTime::forComponents(1872, 1, 1, 0, 0, 0);
  assertTrue(dt.isError());

  // bad year
  dt = LocalDateTime::forComponents(2128, 1, 1, 0, 0, 0);
  assertTrue(dt.isError());

  // bad year
  dt = LocalDateTime::forComponents(9999, 1, 1, 0, 0, 0);
  assertTrue(dt.isError());

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
  LocalDateTime dt;

  // 1931-12-13 20:45:52Z, smalltest datetime using int32_t from AceTime Epoch.
  // Let's use +1 of that since INT_MIN will be used to indicate an error.
  dt = LocalDateTime::forComponents(1931, 12, 13, 20, 45, 53);
  assertEqual((acetime_t) -24856, dt.toEpochDays());
  assertEqual((acetime_t) -13899, dt.toUnixDays());
  assertEqual((acetime_t) (INT32_MIN + 1), dt.toEpochSeconds());
  assertEqual(LocalDate::kSunday, dt.dayOfWeek());

  // 2000-01-01 00:00:00Z Saturday
  dt = LocalDateTime::forComponents(2000, 1, 1, 0, 0, 0);
  assertEqual((acetime_t) 0, dt.toEpochDays());
  assertEqual((acetime_t) 10957, dt.toUnixDays());
  assertEqual((acetime_t) 0, dt.toEpochSeconds());
  assertEqual(LocalDate::kSaturday, dt.dayOfWeek());

  // 2000-01-02 00:00:00Z Sunday
  dt = LocalDateTime::forComponents(2000, 1, 2, 0, 0, 0);
  assertEqual((acetime_t) 1, dt.toEpochDays());
  assertEqual((acetime_t) 10958, dt.toUnixDays());
  assertEqual((acetime_t) 86400, dt.toEpochSeconds());
  assertEqual(LocalDate::kSunday, dt.dayOfWeek());

  // 2000-02-29 00:00:00Z Tuesday
  dt = LocalDateTime::forComponents(2000, 2, 29, 0, 0, 0);
  assertEqual((acetime_t) 59, dt.toEpochDays());
  assertEqual((acetime_t) 11016, dt.toUnixDays());
  assertEqual((acetime_t) 86400 * 59, dt.toEpochSeconds());
  assertEqual(LocalDate::kTuesday, dt.dayOfWeek());

  // 2018-01-01 00:00:00Z Monday
  dt = LocalDateTime::forComponents(2018, 1, 1, 0, 0, 0);
  assertEqual((acetime_t) 6575, dt.toEpochDays());
  assertEqual((acetime_t) 17532, dt.toUnixDays());
  assertEqual(6575 * (acetime_t) 86400, dt.toEpochSeconds());
  assertEqual(LocalDate::kMonday, dt.dayOfWeek());

  // 2038-01-19 03:14:07Z (largest value using Unix Epoch)
  dt = LocalDateTime::forComponents(2038, 1, 19, 3, 14, 7);
  assertEqual((acetime_t) 13898, dt.toEpochDays());
  assertEqual((acetime_t) 24855, dt.toUnixDays());
  assertEqual((acetime_t) 1200798847, dt.toEpochSeconds());
  assertEqual(LocalDate::kTuesday, dt.dayOfWeek());

  // 2068-01-19 03:14:06Z (one second before largest AceTime Epoch)
  dt = LocalDateTime::forComponents(2068, 1, 19, 3, 14, 6);
  assertEqual((acetime_t) 24855, dt.toEpochDays());
  assertEqual((acetime_t) 35812, dt.toUnixDays());
  assertEqual((acetime_t) (INT32_MAX - 1), dt.toEpochSeconds());
  assertEqual(LocalDate::kThursday, dt.dayOfWeek());

  // 2068-01-19 03:14:07Z (largest value for AceTime Epoch).
  dt = LocalDateTime::forComponents(2068, 1, 19, 3, 14, 7);
  assertEqual((acetime_t) 24855, dt.toEpochDays());
  assertEqual((acetime_t) 35812, dt.toUnixDays());
  assertEqual((acetime_t) INT32_MAX, dt.toEpochSeconds());
  assertEqual(LocalDate::kThursday, dt.dayOfWeek());
}

test(LocalDateTimeTest, toAndForUnixSeconds) {
  LocalDateTime dt;
  LocalDateTime udt;

  // 1931-12-13 20:45:52Z, smalltest datetime using int32_t from AceTime Epoch.
  // Let's use +1 of that since INT_MIN will be used to indicate an error.
  dt = LocalDateTime::forComponents(1931, 12, 13, 20, 45, 53);
  assertEqual((acetime_t) -1200798847, dt.toUnixSeconds());
  udt = LocalDateTime::forUnixSeconds(dt.toUnixSeconds());
  assertTrue(dt == udt);

  // 1970-01-01 00:00:00Z
  dt = LocalDateTime::forComponents(1970, 1, 1, 0, 0, 0);
  assertEqual((acetime_t) 0, dt.toUnixSeconds());
  udt = LocalDateTime::forUnixSeconds(dt.toUnixSeconds());
  assertTrue(dt == udt);

  // 2000-01-01 00:00:00Z
  dt = LocalDateTime::forComponents(2000, 1, 1, 0, 0, 0);
  assertEqual((acetime_t) 946684800, dt.toUnixSeconds());
  udt = LocalDateTime::forUnixSeconds(dt.toUnixSeconds());
  assertTrue(dt == udt);

  // 2018-01-01 00:00:00Z
  dt = LocalDateTime::forComponents(2018, 1, 1, 0, 0, 0);
  assertEqual((acetime_t) 1514764800, dt.toUnixSeconds());
  udt = LocalDateTime::forUnixSeconds(dt.toUnixSeconds());
  assertTrue(dt == udt);

  // 2038-01-19 03:14:06Z (largest value - 1 using Unix Epoch)
  dt = LocalDateTime::forComponents(2038, 1, 19, 3, 14, 6);
  assertEqual((acetime_t) (INT32_MAX - 1), dt.toUnixSeconds());
  udt = LocalDateTime::forUnixSeconds(dt.toUnixSeconds());
  assertTrue(dt == udt);
}

test(LocalDateTimeTest, forEpochSeconds) {
  // 2029-12-31 23:59:59Z Monday
  LocalDateTime dt = LocalDateTime::forEpochSeconds(
      10958 * (acetime_t) 86400 - 1);

  assertEqual((int16_t) 2029, dt.year());
  assertEqual(29, dt.yearTiny());
  assertEqual(12, dt.month());
  assertEqual(31, dt.day());
  assertEqual(23, dt.hour());
  assertEqual(59, dt.minute());
  assertEqual(59, dt.second());
  assertEqual(LocalDate::kMonday, dt.dayOfWeek());
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

  dt.yearTiny(19); // 2019-02-02 23:40:03+00:45, changes dayOfWeek
  assertEqual(LocalDate::kSaturday, dt.dayOfWeek());

  dt.year(2020); // 2020-02-02 23:40:03+00:45, changes dayOfWeek
  assertEqual(LocalDate::kSunday, dt.dayOfWeek());
}

test(LocalDateTimeTest, forDateString) {
  // exact ISO8601 format
  LocalDateTime dt = LocalDateTime::forDateString(F("2018-08-31T13:48:01"));
  assertFalse(dt.isError());
  assertEqual((int16_t) 2018, dt.year());
  assertEqual(18, dt.yearTiny());
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
  assertEqual(18, dt.yearTiny());
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
