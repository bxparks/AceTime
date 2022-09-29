#line 2 "OffsetDateTimeTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace ace_time;

test(OffsetDateTimeTest, accessors_mutators) {
  // accessors
  OffsetDateTime dt = OffsetDateTime::forComponents(2001, 2, 3, 4, 5, 6,
      TimeOffset());
  assertEqual((int16_t) 2001, dt.year());
  assertEqual(2, dt.month());
  assertEqual(3, dt.day());
  assertEqual(4, dt.hour());
  assertEqual(5, dt.minute());
  assertEqual(6, dt.second());
  assertEqual(0, dt.timeOffset().toMinutes());
  assertEqual(0, dt.fold());

  // mutators
  dt.year(2011);
  dt.month(12);
  dt.day(13);
  dt.hour(14);
  dt.minute(15);
  dt.second(16);
  dt.timeOffset(TimeOffset::forMinutes(17));
  dt.fold(1);
  assertEqual(2011, dt.year());
  assertEqual(12, dt.month());
  assertEqual(13, dt.day());
  assertEqual(14, dt.hour());
  assertEqual(15, dt.minute());
  assertEqual(16, dt.second());
  assertEqual(17, dt.timeOffset().toMinutes());
  assertEqual(1, dt.fold());
}

test(OffsetDateTimeTest, constructor_with_fold) {
  OffsetDateTime dt = OffsetDateTime::forComponents(
      2001, 2, 3, 4, 5, 6, TimeOffset(), 1 /*fold*/);
  assertEqual((int16_t) 2001, dt.year());
  assertEqual(2, dt.month());
  assertEqual(3, dt.day());
  assertEqual(4, dt.hour());
  assertEqual(5, dt.minute());
  assertEqual(6, dt.second());
  assertEqual(0, dt.timeOffset().toMinutes());
  assertEqual(1, dt.fold());
}

test(OffsetDateTimeTest, invalidSeconds) {
  OffsetDateTime dt = OffsetDateTime::forEpochSeconds(
      LocalDate::kInvalidEpochSeconds, TimeOffset());
  assertTrue(dt.isError());
  assertEqual(LocalDate::kInvalidEpochSeconds, dt.toEpochSeconds());
  assertEqual(LocalDate::kInvalidEpochDays, dt.toEpochDays());
}

test(OffsetDateTimeTest, invalidTimeOffset) {
  OffsetDateTime dt = OffsetDateTime::forEpochSeconds(
      0, TimeOffset::forError());
  assertTrue(dt.isError());
  assertEqual(LocalDate::kInvalidEpochSeconds, dt.toEpochSeconds());
  assertEqual(LocalDate::kInvalidEpochDays, dt.toEpochDays());
}

test(OffsetDateTimeTest, forError) {
  OffsetDateTime dt = OffsetDateTime::forError();
  assertTrue(dt.isError());
}

test(OffsetDateTimeTest, isError) {
  // 2018-01-01 00:00:00Z
  OffsetDateTime dt = OffsetDateTime::forComponents(2018, 1, 1, 0, 0, 0,
      TimeOffset());
  assertFalse(dt.isError());

  // bad year
  dt = OffsetDateTime::forComponents(0, 0, 1, 0, 0, 0, TimeOffset());
  assertTrue(dt.isError());

  // bad year
  dt = OffsetDateTime::forComponents(1872, 0, 1, 0, 0, 0, TimeOffset());
  assertTrue(dt.isError());

  // bad year
  dt = OffsetDateTime::forComponents(2128, 255, 1, 0, 0, 0, TimeOffset());
  assertTrue(dt.isError());

  // bad month
  dt = OffsetDateTime::forComponents(2018, 0, 1, 0, 0, 0, TimeOffset());
  assertTrue(dt.isError());

  // bad month
  dt = OffsetDateTime::forComponents(2018, 255, 1, 0, 0, 0, TimeOffset());
  assertTrue(dt.isError());

  // bad day
  dt = OffsetDateTime::forComponents(2018, 1, 0, 0, 0, 0, TimeOffset());
  assertTrue(dt.isError());

  // bad day
  dt = OffsetDateTime::forComponents(2018, 1, 255, 0, 0, 0, TimeOffset());
  assertTrue(dt.isError());

  // bad hour
  dt = OffsetDateTime::forComponents(2018, 1, 1, 255, 0, 0, TimeOffset());
  assertTrue(dt.isError());

  // bad minute
  dt = OffsetDateTime::forComponents(2018, 1, 1, 0, 255, 0, TimeOffset());
  assertTrue(dt.isError());

  // bad second
  dt = OffsetDateTime::forComponents(2018, 1, 1, 0, 0, 255, TimeOffset());
  assertTrue(dt.isError());
}

test(OffsetDateTimeTest, forComponents) {
  OffsetDateTime dt;

  // 1931-12-13 20:45:52Z, smalltest datetime using int32_t from AceTime Epoch.
  // Let's use +1 of that since INT_MIN will be used to indicate an error.
  dt = OffsetDateTime::forComponents(1931, 12, 13, 20, 45, 53, TimeOffset());
  assertEqual((int32_t) -24856, dt.toEpochDays());
  assertEqual((int32_t) -13899, dt.toUnixDays());
  assertEqual((acetime_t) (INT32_MIN + 1), dt.toEpochSeconds());
  assertEqual(LocalDate::kSunday, dt.dayOfWeek());

  // 2000-01-01 00:00:00Z Saturday
  dt = OffsetDateTime::forComponents(2000, 1, 1, 0, 0, 0, TimeOffset());
  assertEqual((int32_t) 0, dt.toEpochDays());
  assertEqual((int32_t) 10957, dt.toUnixDays());
  assertEqual((acetime_t) 0, dt.toEpochSeconds());
  assertEqual(LocalDate::kSaturday, dt.dayOfWeek());

  // 2000-01-02 00:00:00Z Sunday
  dt = OffsetDateTime::forComponents(2000, 1, 2, 0, 0, 0, TimeOffset());
  assertEqual((int32_t) 1, dt.toEpochDays());
  assertEqual((int32_t) 10958, dt.toUnixDays());
  assertEqual((acetime_t) 86400, dt.toEpochSeconds());
  assertEqual(LocalDate::kSunday, dt.dayOfWeek());

  // 2000-02-29 00:00:00Z Tuesday
  dt = OffsetDateTime::forComponents(2000, 2, 29, 0, 0, 0, TimeOffset());
  assertEqual((int32_t) 59, dt.toEpochDays());
  assertEqual((int32_t) 11016, dt.toUnixDays());
  assertEqual((acetime_t) 86400 * 59, dt.toEpochSeconds());
  assertEqual(LocalDate::kTuesday, dt.dayOfWeek());

  // 2018-01-01 00:00:00Z Monday
  dt = OffsetDateTime::forComponents(2018, 1, 1, 0, 0, 0, TimeOffset());
  assertEqual((int32_t) 6575, dt.toEpochDays());
  assertEqual((int32_t) 17532, dt.toUnixDays());
  assertEqual(6575 * (acetime_t) 86400, dt.toEpochSeconds());
  assertEqual(LocalDate::kMonday, dt.dayOfWeek());

  // 2038-01-19 03:14:07Z (largest value using 32-bit Unix seconds)
  dt = OffsetDateTime::forComponents(2038, 1, 19, 3, 14, 7, TimeOffset());
  assertEqual((int32_t) 13898, dt.toEpochDays());
  assertEqual((int32_t) 24855, dt.toUnixDays());
  assertEqual((acetime_t) 1200798847, dt.toEpochSeconds());
  assertEqual(LocalDate::kTuesday, dt.dayOfWeek());

  // 2068-01-19 03:14:06Z (one second before largest AceTime Epoch).
  dt = OffsetDateTime::forComponents(2068, 1, 19, 3, 14, 6, TimeOffset());
  assertEqual((int32_t) 24855, dt.toEpochDays());
  assertEqual((int32_t) 35812, dt.toUnixDays());
  assertEqual((acetime_t) (INT32_MAX - 1), dt.toEpochSeconds());
  assertEqual(LocalDate::kThursday, dt.dayOfWeek());

  // 2068-01-19 03:14:07Z (largest AceTime Epoch).
  dt = OffsetDateTime::forComponents(2068, 1, 19, 3, 14, 7, TimeOffset());
  assertEqual((int32_t) 24855, dt.toEpochDays());
  assertEqual((int32_t) 35812, dt.toUnixDays());
  assertEqual((acetime_t) INT32_MAX, dt.toEpochSeconds());
  assertEqual(LocalDate::kThursday, dt.dayOfWeek());
}

test(OffsetDateTimeTest, forComponents_withOffset) {
  OffsetDateTime dt;

  // 2018-01-01 00:00:00+00:15 Monday
  dt = OffsetDateTime::forComponents(
      2018, 1, 1, 0, 0, 0, TimeOffset::forMinutes(15));
  assertEqual((int32_t) 6574, dt.toEpochDays());
  assertEqual((int32_t) 17531, dt.toUnixDays());
  assertEqual(6575 * (acetime_t) 86400 - 15*60, dt.toEpochSeconds());
  assertEqual(LocalDate::kMonday, dt.dayOfWeek());
}

test(OffsetDateTimeTest, toAndForUnixSeconds64) {
  OffsetDateTime dt;
  OffsetDateTime udt;

  // Verify error sentinel.
  dt = OffsetDateTime::forUnixSeconds64(
      LocalDate::kInvalidEpochSeconds64, TimeOffset());
  assertTrue(dt.isError());

  // Verify that 64-bit unixSeconds allows dates beyond 32-bit limit.
  // 1970 - 1770 = 200 years
  //      = 200 * 365 + (200/4) leap years - 2 (1800, 1900 are not leap)
  //      = 73048 days
  dt = OffsetDateTime::forComponents(1770, 1, 1, 0, 0, 0, TimeOffset());
  assertEqual((int64_t) -73048 * 86400, dt.toUnixSeconds64());
  udt = OffsetDateTime::forUnixSeconds64(
      (int64_t) -73048 * 86400, TimeOffset());
  assertTrue(dt == udt);

  // 1931-12-13 20:45:52Z, smalltest datetime using int32_t from AceTime Epoch.
  // Let's use +1 of that since INT_MIN will be used to indicate an error.
  dt = OffsetDateTime::forComponents(1931, 12, 13, 20, 45, 53, TimeOffset());
  assertEqual((int64_t) -1200798847, dt.toUnixSeconds64());
  udt = OffsetDateTime::forUnixSeconds64(dt.toUnixSeconds64(), TimeOffset());
  assertTrue(dt == udt);

  // 1970-01-01 00:00:00Z
  dt = OffsetDateTime::forComponents(1970, 1, 1, 0, 0, 0, TimeOffset());
  assertEqual((int64_t) 0, dt.toUnixSeconds64());
  udt = OffsetDateTime::forUnixSeconds64(dt.toUnixSeconds64(), TimeOffset());
  assertTrue(dt == udt);

  // 2000-01-01 00:00:00Z
  dt = OffsetDateTime::forComponents(2000, 1, 1, 0, 0, 0, TimeOffset());
  assertEqual((int64_t) 946684800, dt.toUnixSeconds64());
  udt = OffsetDateTime::forUnixSeconds64(dt.toUnixSeconds64(), TimeOffset());
  assertTrue(dt == udt);

  // 2018-01-01 00:00:00Z
  dt = OffsetDateTime::forComponents(2018, 1, 1, 0, 0, 0, TimeOffset());
  assertEqual((int64_t) 1514764800, dt.toUnixSeconds64());
  udt = OffsetDateTime::forUnixSeconds64(dt.toUnixSeconds64(), TimeOffset());
  assertTrue(dt == udt);

  // 2018-08-30T06:45:01-07:00
  dt = OffsetDateTime::forComponents(2018, 8, 30, 6, 45, 1,
      TimeOffset::forHours(-7));
  assertEqual((int64_t) 1535636701, dt.toUnixSeconds64());
  udt = OffsetDateTime::forUnixSeconds64(dt.toUnixSeconds64(),
      TimeOffset::forHours(-7));
  assertTrue(dt == udt);

  // 2038-01-19 03:14:06Z (largest value - 1 using 32-bit Unix seconds)
  dt = OffsetDateTime::forComponents(2038, 1, 19, 3, 14, 6,
      TimeOffset());
  assertEqual((int64_t) (INT32_MAX - 1), dt.toUnixSeconds64());
  udt = OffsetDateTime::forUnixSeconds64(dt.toUnixSeconds64(), TimeOffset());
  assertTrue(dt == udt);

  // 2038-01-19 03:14:08Z (largest value + 1 using 32-bit Unix seconds)
  dt = OffsetDateTime::forComponents(2038, 1, 19, 3, 14, 8, TimeOffset());
  assertEqual((int64_t) INT32_MAX + 1, dt.toUnixSeconds64());
  udt = OffsetDateTime::forUnixSeconds64(dt.toUnixSeconds64(), TimeOffset());
  assertTrue(dt == udt);

  // 2068-01-19 03:14:07Z (largest value for 32-bit AceTime seconds) should work
  // with 64-bit Unix seconds.
  dt = OffsetDateTime::forComponents(2068, 1, 19, 3, 14, 7, TimeOffset());
  assertEqual((int64_t) 3094168447, dt.toUnixSeconds64());
  udt = OffsetDateTime::forUnixSeconds64(dt.toUnixSeconds64(), TimeOffset());
  assertTrue(dt == udt);

  // Verify that year 2170 works just fine with 64-bit unix seconds.
  // 2170 - 1970 = 200 years
  //    = 73049 days, instead of 73048 days, because 2000 was a leap year.
  dt = OffsetDateTime::forComponents(2170, 1, 1, 0, 0, 0, TimeOffset());
  assertEqual((int64_t) 73049 * 86400, dt.toUnixSeconds64());
  udt = OffsetDateTime::forUnixSeconds64(
      (int64_t) 73049 * 86400, TimeOffset());
  assertTrue(dt == udt);
}

test(OffsetDateTimeTest, toAndForUnixSeconds64_withOffset) {
  OffsetDateTime dt;
  OffsetDateTime udt;

  TimeOffset offset = TimeOffset::forMinutes(15);
  // 1970-01-01 00:00:00+00:15
  dt = OffsetDateTime::forComponents(1970, 1, 1, 0, 0, 0, offset);
  assertEqual((int64_t) -15 * 60, dt.toUnixSeconds64());
  udt = OffsetDateTime::forUnixSeconds64(dt.toUnixSeconds64(), offset);
  assertTrue(dt == udt);
}

test(OffsetDateTimeTest, forEpochSeconds) {
  // 2029-12-31 23:59:59Z Monday
  OffsetDateTime dt = OffsetDateTime::forEpochSeconds(
      10958 * (acetime_t) 86400 - 1, TimeOffset());

  assertEqual((int16_t) 2029, dt.year());
  assertEqual(12, dt.month());
  assertEqual(31, dt.day());
  assertEqual(23, dt.hour());
  assertEqual(59, dt.minute());
  assertEqual(59, dt.second());
  assertEqual(LocalDate::kMonday, dt.dayOfWeek());

  // 2029-12-31 15:59:59-08:00 Monday
  TimeOffset offset = TimeOffset::forHours(-8); // UTC-08:00
  dt = OffsetDateTime::forEpochSeconds(10958 * (acetime_t) 86400 - 1, offset);
  assertEqual((int16_t) 2029, dt.year());
  assertEqual(12, dt.month());
  assertEqual(31, dt.day());
  assertEqual(15, dt.hour());
  assertEqual(59, dt.minute());
  assertEqual(59, dt.second());
  assertEqual(LocalDate::kMonday, dt.dayOfWeek());
  assertEqual(0, dt.fold());
}

test(OffsetDateTimeTest, forEpochSeconds_withFold) {
  // 2029-12-31 23:59:59Z Monday
  OffsetDateTime dt = OffsetDateTime::forEpochSeconds(
      10958 * (acetime_t) 86400 - 1, TimeOffset(), 1 /*fold*/);

  assertEqual((int16_t) 2029, dt.year());
  assertEqual(12, dt.month());
  assertEqual(31, dt.day());
  assertEqual(23, dt.hour());
  assertEqual(59, dt.minute());
  assertEqual(59, dt.second());
  assertEqual(LocalDate::kMonday, dt.dayOfWeek());
  assertEqual(1, dt.fold());
}

test(OffsetDateTimeTest, convertToTimeOffset) {
  OffsetDateTime a = OffsetDateTime::forComponents(2018, 1, 1, 12, 0, 0,
      TimeOffset());
  OffsetDateTime b = a.convertToTimeOffset(TimeOffset::forHours(-7));

  assertEqual((int16_t) 2018, b.year());
  assertEqual(1, b.month());
  assertEqual(1, b.day());
  assertEqual(5, b.hour());
  assertEqual(0, b.minute());
  assertEqual(0, b.second());
  assertEqual(-7*60, b.timeOffset().toMinutes());
}

test(OffsetDateTimeTest, compareTo) {
  OffsetDateTime a = OffsetDateTime::forComponents(2018, 1, 1, 12, 0, 0,
      TimeOffset());
  OffsetDateTime b = OffsetDateTime::forComponents(2018, 1, 1, 12, 0, 0,
      TimeOffset());
  assertEqual(a.compareTo(b), 0);
  assertTrue(a == b);
  assertFalse(a != b);

  a = OffsetDateTime::forComponents(2018, 1, 1, 12, 0, 0, TimeOffset());
  b = OffsetDateTime::forComponents(2018, 1, 1, 12, 0, 1, TimeOffset());
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);

  a = OffsetDateTime::forComponents(2018, 1, 1, 12, 0, 0, TimeOffset());
  b = OffsetDateTime::forComponents(2018, 1, 1, 12, 1, 0, TimeOffset());
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);

  a = OffsetDateTime::forComponents(2018, 1, 1, 11, 0, 0,
      TimeOffset());
  b = OffsetDateTime::forComponents(2018, 1, 1, 12, 0, 0,
      TimeOffset::forMinutes(15));
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);

  a = OffsetDateTime::forComponents(2018, 1, 1, 12, 0, 0, TimeOffset());
  b = OffsetDateTime::forComponents(2018, 1, 2, 12, 0, 0, TimeOffset());
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);

  a = OffsetDateTime::forComponents(2018, 1, 1, 12, 0, 0, TimeOffset());
  b = OffsetDateTime::forComponents(2018, 2, 1, 12, 0, 0, TimeOffset());
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);

  a = OffsetDateTime::forComponents(2018, 1, 1, 12, 0, 0, TimeOffset());
  b = OffsetDateTime::forComponents(2019, 1, 1, 12, 0, 0, TimeOffset());
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);

  // 2018-1-1 12:00:00+01:00
  a = OffsetDateTime::forComponents(2018, 1, 1, 12, 0, 0,
      TimeOffset::forHours(1));
  // 2018-1-1 12:00:00-08:00
  b = OffsetDateTime::forComponents(2018, 1, 1, 12, 0, 0,
      TimeOffset::forHours(-8));
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);
}

test(OffsetDateTimeTest, dayOfWeek) {
  // 2018-01-01 00:00:00Z Monday
  OffsetDateTime dt = OffsetDateTime::forComponents(2018, 1, 1, 0, 0, 0,
      TimeOffset());
  assertEqual(LocalDate::kMonday, dt.dayOfWeek());

  dt.hour(23); // 2018-01-01 23:00:00Z, no change to dayOfWeek
  assertEqual(LocalDate::kMonday, dt.dayOfWeek());

  dt.minute(40); // 2018-01-01 23:40:00Z, no change to dayOfWeek
  assertEqual(LocalDate::kMonday, dt.dayOfWeek());

  dt.second(3); // 2018-01-01 23:40:03Z, no change to dayOfWeek
  assertEqual(LocalDate::kMonday, dt.dayOfWeek());

  // 2018-01-01 23:40:03+00:45, no change to dayOfWeek
  dt.timeOffset(TimeOffset::forMinutes(45));
  assertEqual(LocalDate::kMonday, dt.dayOfWeek());

  dt.day(2); // 2018-01-02 23:40:03+00:45, changes dayOfWeek
  assertEqual(LocalDate::kTuesday, dt.dayOfWeek());

  dt.month(2); // 2018-02-02 23:40:03+00:45, changes dayOfWeek
  assertEqual(LocalDate::kFriday, dt.dayOfWeek());

  dt.year(2020); // 2020-02-02 23:40:03+00:45, changes dayOfWeek
  assertEqual(LocalDate::kSunday, dt.dayOfWeek());
}

test(OffsetDateTimeTest, forDateString) {
  // exact ISO8601 format
  auto dt = OffsetDateTime::forDateString(F("2018-08-31T13:48:01-07:00"));
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
  dt = OffsetDateTime::forDateString(F("2018/08/31 13#48#01+07#00"));
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

test(OffsetDateTimeTest, forDateString_errors) {
  // empty string, too short
  OffsetDateTime dt = OffsetDateTime::forDateString("");
  assertTrue(dt.isError());

  // not enough components
  dt = OffsetDateTime::forDateString(F("2018-08-31"));
  assertTrue(dt.isError());

  // too long
  dt = OffsetDateTime::forDateString(F("2018-08-31T13:48:01-07:00X"));
  assertTrue(dt.isError());

  // too short
  dt = OffsetDateTime::forDateString(F("2018-08-31T13:48:01-07:0"));
  assertTrue(dt.isError());

  // missing UTC
  dt = OffsetDateTime::forDateString(F("2018-08-31T13:48:01"));
  assertTrue(dt.isError());

  // parser cares about the +/- in front of the UTC offset
  dt = OffsetDateTime::forDateString(F("2018-08-31 13:48:01&07:00"));
  assertTrue(dt.isError());
}

//---------------------------------------------------------------------------
// data_time_mutation
//---------------------------------------------------------------------------

test(OffsetDateTimeMutationTest, incrementYear) {
  OffsetDateTime dt = OffsetDateTime::forComponents(2001, 2, 3, 4, 5, 6,
      TimeOffset());
  assertEqual((int16_t) 2001, dt.year());
  assertEqual(2, dt.month());
  assertEqual(3, dt.day());
  assertEqual(4, dt.hour());
  assertEqual(5, dt.minute());
  assertEqual(6, dt.second());
  assertEqual(0, dt.timeOffset().toMinutes());

  offset_date_time_mutation::incrementYear(dt);
  assertEqual((int16_t) 2002, dt.year());
  assertEqual(2, dt.month());
  assertEqual(3, dt.day());
  assertEqual(4, dt.hour());
  assertEqual(5, dt.minute());
  assertEqual(6, dt.second());
  assertEqual(0, dt.timeOffset().toMinutes());

  dt = OffsetDateTime::forComponents(2099, 2, 3, 4, 5, 6, TimeOffset());
  offset_date_time_mutation::incrementYear(dt);
  assertEqual((int16_t) 2000, dt.year());
  assertEqual(2, dt.month());
  assertEqual(3, dt.day());
  assertEqual(4, dt.hour());
  assertEqual(5, dt.minute());
  assertEqual(6, dt.second());
  assertEqual(0, dt.timeOffset().toMinutes());
}

test(OffsetDateTimeMutationTest, incrementMonth) {
  OffsetDateTime dt = OffsetDateTime::forComponents(2001, 2, 3, 4, 5, 6,
      TimeOffset());
  offset_date_time_mutation::incrementMonth(dt);
  assertEqual((int16_t) 2001, dt.year());
  assertEqual(3, dt.month());
  assertEqual(3, dt.day());
  assertEqual(4, dt.hour());
  assertEqual(5, dt.minute());
  assertEqual(6, dt.second());
  assertEqual(0, dt.timeOffset().toMinutes());
}

test(OffsetDateTimeMutationTest, incrementDay) {
  OffsetDateTime dt = OffsetDateTime::forComponents(2001, 2, 3, 4, 5, 6,
      TimeOffset());
  offset_date_time_mutation::incrementDay(dt);
  assertEqual((int16_t) 2001, dt.year());
  assertEqual(2, dt.month());
  assertEqual(4, dt.day());
  assertEqual(4, dt.hour());
  assertEqual(5, dt.minute());
  assertEqual(6, dt.second());
  assertEqual(0, dt.timeOffset().toMinutes());
}

test(OffsetDateTimeMutationTest, incrementHour) {
  OffsetDateTime dt = OffsetDateTime::forComponents(2001, 2, 3, 4, 5, 6,
      TimeOffset());
  offset_date_time_mutation::incrementHour(dt);
  assertEqual((int16_t) 2001, dt.year());
  assertEqual(2, dt.month());
  assertEqual(3, dt.day());
  assertEqual(5, dt.hour());
  assertEqual(5, dt.minute());
  assertEqual(6, dt.second());
  assertEqual(0, dt.timeOffset().toMinutes());
}

test(OffsetDateTimeMutationTest, incrementMinute) {
  OffsetDateTime dt = OffsetDateTime::forComponents(2001, 2, 3, 4, 5, 6,
      TimeOffset());
  offset_date_time_mutation::incrementMinute(dt);
  assertEqual((int16_t) 2001, dt.year());
  assertEqual(2, dt.month());
  assertEqual(3, dt.day());
  assertEqual(4, dt.hour());
  assertEqual(6, dt.minute());
  assertEqual(6, dt.second());
  assertEqual(0, dt.timeOffset().toMinutes());
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
