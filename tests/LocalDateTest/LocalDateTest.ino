#line 2 "LocalDateTest.ino"

#include <AUnit.h>
#include <AceTime.h>
#include <ace_time/testing/EpochYearContext.h>

using namespace ace_time;

//---------------------------------------------------------------------------
// LocalDate
//---------------------------------------------------------------------------

test(LocalDateTest, year_limits) {
  assertLess(LocalDate::kInvalidYear, LocalDate::kMinYear);
  assertLess(LocalDate::kInvalidYear, basic::ZoneRule::kMinYear);
  assertLess(LocalDate::kInvalidYear, extended::ZoneRule::kMinYear);

  assertLess(LocalDate::kMinYear, LocalDate::kMaxYear);
  assertLess(LocalDate::kMinYear, LocalDate::kMaxYear);

  assertLess(basic::ZoneRule::kMinYear, basic::ZoneRule::kMaxYear);
  assertLess(extended::ZoneRule::kMinYear, extended::ZoneRule::kMaxYear);

  assertLess(basic::ZoneRule::kMaxYear, basic::ZoneEra::kMaxUntilYear);
  assertLess(extended::ZoneRule::kMaxYear, extended::ZoneEra::kMaxUntilYear);
}

test(LocalDateTest, year_range) {
  // Not valid
  assertTrue(LocalDate::forComponents(INT16_MIN, 1, 1).isError());
  assertTrue(LocalDate::forComponents(-1, 1, 1).isError());

  // kMinYear allowed
  assertFalse(LocalDate::forComponents(0, 1, 1).isError());

  // first valid year allowed
  assertFalse(LocalDate::forComponents(1, 1, 1).isError());

  // largest valid FROM or TO year ("max"), allowed
  assertFalse(LocalDate::forComponents(9999, 1, 1).isError());

  // largest valid UNTIL year ("-"), allowed, kMaxYear
  assertFalse(LocalDate::forComponents(10000, 1, 1).isError());

  // Not valid
  assertTrue(LocalDate::forComponents(LocalDate::kMaxYear + 1, 1, 1).isError());
  assertTrue(LocalDate::forComponents(INT16_MAX, 1, 1).isError());
}

test(LocalDateTest, month_range) {
  assertTrue(LocalDate::forComponents(2000, 0, 1).isError());
  assertTrue(LocalDate::forComponents(2000, 13, 1).isError());
}

test(LocalDateTest, day_range) {
  assertTrue(LocalDate::forComponents(2000, 1, 0).isError());
  assertTrue(LocalDate::forComponents(2000, 1, 32).isError());
}

test(LocalDateTest, accessors) {
  LocalDate ld = LocalDate::forComponents(2001, 2, 3);
  assertEqual((int16_t) 2001, ld.year());
  assertEqual(2, ld.month());
  assertEqual(3, ld.day());
}

// Verify that toEpochDays()/forEpochDays() and
// toEpochSeconds()/forEpochSeconds() support round trip conversions when when
// isError()==true.
test(LocalDateTest, forError_roundTrip) {
  LocalDate ld;

  ld = LocalDate::forError();
  assertTrue(ld.isError());
  assertEqual(LocalDate::kInvalidEpochDays, ld.toEpochDays());
  assertEqual(LocalDate::kInvalidEpochSeconds, ld.toEpochSeconds());

  ld = LocalDate::forEpochDays(LocalDate::kInvalidEpochDays);
  assertTrue(ld.isError());

  ld = LocalDate::forEpochSeconds(LocalDate::kInvalidEpochSeconds);
  assertTrue(ld.isError());
}

test(LocalDateTest, dayOfWeek) {
  // year 1900 (not leap year due to every 100 rule)
  assertEqual(LocalDate::kWednesday,
      LocalDate::forComponents(1900, 2, 28).dayOfWeek());
  assertEqual(LocalDate::kThursday,
      LocalDate::forComponents(1900, 3, 1).dayOfWeek());

  // year 1999 was not a leap year
  assertEqual(LocalDate::kFriday,
      LocalDate::forComponents(1999, 1, 1).dayOfWeek());
  assertEqual(LocalDate::kSunday,
      LocalDate::forComponents(1999, 1, 31).dayOfWeek());

  // year 2000 (leap year due to every 400 rule)
  assertEqual(LocalDate::kSaturday,
      LocalDate::forComponents(2000, 1, 1).dayOfWeek());
  assertEqual(LocalDate::kMonday,
      LocalDate::forComponents(2000, 1, 31).dayOfWeek());

  assertEqual(LocalDate::kTuesday,
      LocalDate::forComponents(2000, 2, 1).dayOfWeek());
  assertEqual(LocalDate::kTuesday,
      LocalDate::forComponents(2000, 2, 29).dayOfWeek());

  assertEqual(LocalDate::kWednesday,
      LocalDate::forComponents(2000, 3, 1).dayOfWeek());
  assertEqual(LocalDate::kFriday,
      LocalDate::forComponents(2000, 3, 31).dayOfWeek());

  assertEqual(LocalDate::kSaturday,
      LocalDate::forComponents(2000, 4, 1).dayOfWeek());
  assertEqual(LocalDate::kSunday,
      LocalDate::forComponents(2000, 4, 30).dayOfWeek());

  assertEqual(LocalDate::kMonday,
      LocalDate::forComponents(2000, 5, 1).dayOfWeek());
  assertEqual(LocalDate::kWednesday,
      LocalDate::forComponents(2000, 5, 31).dayOfWeek());

  assertEqual(LocalDate::kThursday,
      LocalDate::forComponents(2000, 6, 1).dayOfWeek());
  assertEqual(LocalDate::kFriday,
      LocalDate::forComponents(2000, 6, 30).dayOfWeek());

  assertEqual(LocalDate::kSaturday,
      LocalDate::forComponents(2000, 7, 1).dayOfWeek());
  assertEqual(LocalDate::kMonday,
      LocalDate::forComponents(2000, 7, 31).dayOfWeek());

  assertEqual(LocalDate::kTuesday,
      LocalDate::forComponents(2000, 8, 1).dayOfWeek());
  assertEqual(LocalDate::kThursday,
      LocalDate::forComponents(2000, 8, 31).dayOfWeek());

  assertEqual(LocalDate::kFriday,
      LocalDate::forComponents(2000, 9, 1).dayOfWeek());
  assertEqual(LocalDate::kSaturday,
      LocalDate::forComponents(2000, 9, 30).dayOfWeek());

  assertEqual(LocalDate::kSunday,
      LocalDate::forComponents(2000, 10, 1).dayOfWeek());
  assertEqual(LocalDate::kTuesday,
      LocalDate::forComponents(2000, 10, 31).dayOfWeek());

  assertEqual(LocalDate::kWednesday,
      LocalDate::forComponents(2000, 11, 1).dayOfWeek());
  assertEqual(LocalDate::kThursday,
      LocalDate::forComponents(2000, 11, 30).dayOfWeek());

  assertEqual(LocalDate::kFriday,
      LocalDate::forComponents(2000, 12, 1).dayOfWeek());
  assertEqual(LocalDate::kSunday,
      LocalDate::forComponents(2000, 12, 31).dayOfWeek());

  // year 2001
  assertEqual(LocalDate::kMonday,
      LocalDate::forComponents(2001, 1, 1).dayOfWeek());
  assertEqual(LocalDate::kWednesday,
      LocalDate::forComponents(2001, 1, 31).dayOfWeek());

  // year 2004 (leap year)
  assertEqual(LocalDate::kSunday,
      LocalDate::forComponents(2004, 2, 1).dayOfWeek());
  assertEqual(LocalDate::kSunday,
      LocalDate::forComponents(2004, 2, 29).dayOfWeek());
  assertEqual(LocalDate::kMonday,
      LocalDate::forComponents(2004, 3, 1).dayOfWeek());

  // year 2099
  assertEqual(LocalDate::kThursday,
      LocalDate::forComponents(2099, 1, 1).dayOfWeek());
  assertEqual(LocalDate::kThursday,
      LocalDate::forComponents(2099, 12, 31).dayOfWeek());

  // year 2100 (not leap year due to every 100 rule)
  assertEqual(LocalDate::kSunday,
      LocalDate::forComponents(2100, 2, 28).dayOfWeek());
  assertEqual(LocalDate::kMonday,
      LocalDate::forComponents(2100, 3, 1).dayOfWeek());
}

test(LocalDateTest, toAndFromEpochDays) {
  LocalDate ld;

  // Smallest LocalDate in our 16-bit implementation is 0001-01-01
  ld = LocalDate::forComponents(1, 1, 1);
  int32_t epoch1 = (int32_t) ((0 - 2000) / 400) * 146097 + 366;
  assertEqual(epoch1, ld.toEpochDays());
  assertTrue(ld == LocalDate::forEpochDays(epoch1));

  // Smallest LocalDate in an 8-bit implementation
  ld = LocalDate::forComponents(1873, 1, 1);
  assertEqual((int32_t) -46385, ld.toEpochDays());
  assertTrue(ld == LocalDate::forEpochDays(-46385));

  ld = LocalDate::forComponents(1900, 1, 1);
  assertEqual((int32_t) -36524, ld.toEpochDays());
  assertTrue(ld == LocalDate::forEpochDays(-36524));

  // Smallest date using int32_t seconds from AceTime epoch
  ld = LocalDate::forComponents(1931, 12, 14);
  assertEqual((int32_t) -24855, ld.toEpochDays());
  assertTrue(ld == LocalDate::forEpochDays(-24855));

  // AceTime Epoch
  ld = LocalDate::forComponents(2000, 1, 1);
  assertEqual((int32_t) 0, ld.toEpochDays());
  assertTrue(ld == LocalDate::forEpochDays(0));

  ld = LocalDate::forComponents(2000, 2, 29);
  assertEqual((int32_t) 59, ld.toEpochDays());
  assertTrue(ld == LocalDate::forEpochDays(59));

  ld = LocalDate::forComponents(2018, 1, 1);
  assertEqual((int32_t) 6575, ld.toEpochDays());
  assertTrue(ld == LocalDate::forEpochDays(6575));

  // Largest date using int32_t seconds from AceTime epoch
  ld = LocalDate::forComponents(2068, 1, 19);
  assertEqual((int32_t) 24855, ld.toEpochDays());
  assertTrue(ld == LocalDate::forEpochDays(24855));

  // Largest LocalDate in an 8-bit implementation
  ld = LocalDate::forComponents(2127, 12, 31);
  assertEqual((int32_t) 46750, ld.toEpochDays());
  assertTrue(ld == LocalDate::forEpochDays(46750));

  // Largest LocalDate in our 16-bit implementation is 9999-12-31
  ld = LocalDate::forComponents(9999, 12, 31);
  const int32_t epochDays9999 = (int32_t) ((10000 - 2000) / 400) * 146097 - 1;
  assertEqual(epochDays9999, ld.toEpochDays());
  assertTrue(ld == LocalDate::forEpochDays(epochDays9999));
}

// Change currentEpochYear to a different value.
test(LocalDateTest, toAndFromEpochDays_epoch2050) {
  // Change current epoch year to 2100, making the epoch 2100-01-01T00:00:00.
  testing::EpochYearContext context(2050);

  // Verify lower and upper valid year limits.
  assertEqual(LocalDate::epochValidYearLower(), 2000);
  assertEqual(LocalDate::epochValidYearUpper(), 2100);

  // Verify that 2050-01-01 returns epoch days of 0
  LocalDate ld = LocalDate::forComponents(2050, 1, 1);
  assertEqual((int32_t) 0, ld.toEpochDays());
  assertTrue(ld == LocalDate::forEpochDays(0));

  // Verify the smallest LocalDate. The smallest valid epochseconds is
  // (INT32_MIN+1) because INT32_MIN is a sentinel for an Error condition.
  // The complicated expression below is an integer division of a negative
  // number that truncates towards -Infinity.
  ld = LocalDate::forComponents(1981, 12, 13);
  int32_t smallestEpochDays = (LocalDate::kMinEpochSeconds + 1) / 86400 - 1;
  assertEqual(smallestEpochDays, ld.toEpochDays());
  assertTrue(ld == LocalDate::forEpochDays(smallestEpochDays));

  // Verify the largest LocalDate.
  ld = LocalDate::forComponents(2118, 1, 20);
  int32_t largestEpochDays = LocalDate::kMaxEpochSeconds / 86400;
  assertEqual(largestEpochDays, ld.toEpochDays());
  assertTrue(ld == LocalDate::forEpochDays(largestEpochDays));
}

// Change currentEpochYear to a different value.
test(LocalDateTest, toAndFromEpochDays_epoch2100) {
  // Change current epoch year to 2100, so the epoch becomes
  // 2100-01-01T00:00:00.
  testing::EpochYearContext context(2100);

  // Verify lower and upper valid year limits.
  assertEqual(LocalDate::epochValidYearLower(), 2050);
  assertEqual(LocalDate::epochValidYearUpper(), 2150);

  // Verify that 2100-01-01 returns epoch days of 0
  LocalDate ld = LocalDate::forComponents(2100, 1, 1);
  assertEqual((int32_t) 0, ld.toEpochDays());
  assertTrue(ld == LocalDate::forEpochDays(0));

  // Verify the smallest LocalDate. The smallest valid epochseconds is
  // (INT32_MIN+1) because INT32_MIN is a sentinel for an Error condition.
  // The complicated expression below is an integer division of a negative
  // number that truncates towards -Infinity.
  ld = LocalDate::forComponents(2031, 12, 13);
  int32_t smallestEpochDays = (LocalDate::kMinEpochSeconds + 1) / 86400 - 1;
  assertEqual(smallestEpochDays, ld.toEpochDays());
  assertTrue(ld == LocalDate::forEpochDays(smallestEpochDays));

  // Verify the largest LocalDate.
  ld = LocalDate::forComponents(2168, 1, 20);
  int32_t largestEpochDays = LocalDate::kMaxEpochSeconds / 86400;
  assertEqual(largestEpochDays, ld.toEpochDays());
  assertTrue(ld == LocalDate::forEpochDays(largestEpochDays));
}

// Same as toAndFromEpochDays, shifted 30 years
test(LocalDateTest, toAndFromUnixDays) {
  LocalDate ld;

  // Smallest LocalDate in an 8-bit implementation
  ld = LocalDate::forComponents(1873, 1, 1);
  assertEqual((int32_t) -35428, ld.toUnixDays());
  assertTrue(ld == LocalDate::forUnixDays(-35428));

  // Smallest date using int32_t from Unix epoch
  ld = LocalDate::forComponents(1901, 12, 14);
  assertEqual((int32_t) -24855, ld.toUnixDays());
  assertTrue(ld == LocalDate::forUnixDays(-24855));

  // Unix Epoch
  ld = LocalDate::forComponents(1970, 1, 1);
  assertEqual((int32_t) 0, ld.toUnixDays());
  assertTrue(ld == LocalDate::forUnixDays(0));

  // 1970 is not a leap year, whereas 2000 is a leap year
  ld = LocalDate::forComponents(1970, 2, 28);
  assertEqual((int32_t) 58, ld.toUnixDays());
  assertTrue(ld == LocalDate::forUnixDays(58));

  ld = LocalDate::forComponents(1988, 1, 1);
  assertEqual((int32_t) 6574, ld.toUnixDays());
  assertTrue(ld == LocalDate::forUnixDays(6574));

  // Largest date using int32_t from Unix epoch
  ld = LocalDate::forComponents(2038, 1, 19);
  assertEqual((int32_t) 24855, ld.toUnixDays());
  assertTrue(ld == LocalDate::forUnixDays(24855));

  // Largest LocalDate in an 8-bit implementation
  ld = LocalDate::forComponents(2127, 12, 31);
  assertEqual((int32_t) 57707, ld.toUnixDays());
  assertTrue(ld == LocalDate::forUnixDays(57707));
}

test(LocalDateTest, toAndFromEpochSeconds) {
  LocalDate ld;

  // Smallest date with an int32_t seconds from AceTime Epoch is 1931-12-13
  // 20:45:52. The forEpochSeconds() will correctly truncate the partial day
  // *down* towards the to the nearest whole day.
  ld = LocalDate::forComponents(1931, 12, 13);
  assertTrue(ld == LocalDate::forEpochSeconds(INT32_MIN + 1));

  // The smallest whole day that can be represented with an int32_t seconds from
  // AceTime Epoch is 1931-12-14.
  ld = LocalDate::forComponents(1931, 12, 14);
  assertEqual((acetime_t) -24855 * 86400, ld.toEpochSeconds());
  assertTrue(ld == LocalDate::forEpochSeconds((acetime_t) -24855 * 86400));

  ld = LocalDate::forComponents(2000, 1, 1);
  assertEqual((acetime_t) 0, ld.toEpochSeconds());
  assertTrue(ld == LocalDate::forEpochSeconds(0));

  ld = LocalDate::forComponents(2000, 2, 29);
  assertEqual((acetime_t) 59 * 86400, ld.toEpochSeconds());
  assertTrue(ld == LocalDate::forEpochSeconds((acetime_t) 59 * 86400 + 1));

  ld = LocalDate::forComponents(2018, 1, 1);
  assertEqual((acetime_t) 6575 * 86400, ld.toEpochSeconds());
  assertTrue(ld == LocalDate::forEpochSeconds((acetime_t) 6575 * 86400 + 2));

  // Largest date possible using AceTime Epoch Seconds is 2068-01-19 03:14:07.
  ld = LocalDate::forComponents(2068, 1, 19);
  assertEqual((acetime_t) 24855 * 86400, ld.toEpochSeconds());
  assertTrue(ld == LocalDate::forEpochSeconds(
      (acetime_t) 24855 * 86400 + 11647));
}

test(LocalDateTest, toAndFromUnixSeconds64) {
  LocalDate ld;

  // Verify error sentinel.
  ld = LocalDate::forUnixSeconds64(LocalDate::kInvalidUnixSeconds64);
  assertTrue(ld.isError());

  // Verify that 64-bit unixSeconds allows dates beyond 32-bit limit.
  // 1770 to 1970 is 200 years == 200 * 365 + (200/4) leap years - 2 (1800, 1900
  // are not leap) == 73048 days
  ld = LocalDate::forComponents(1770, 1, 1);
  assertEqual((int64_t) -73048 * 86400, ld.toUnixSeconds64());
  assertTrue(ld == LocalDate::forUnixSeconds64((int64_t) -73048 * 86400));

  // The smallest whole day that can be represented with an int32_t from AceTime
  // epoch is 1931-12-14, can't do better with unixSeconds since it uses
  // the Acetime seconds internally.
  ld = LocalDate::forComponents(1931, 12, 14);
  assertEqual((int64_t) -13898 * 86400, ld.toUnixSeconds64());
  assertTrue(ld == LocalDate::forUnixSeconds64((int64_t) -13898 * 86400));

  ld = LocalDate::forComponents(1970, 1, 1);
  assertEqual((int64_t) 0, ld.toUnixSeconds64());
  assertTrue(ld == LocalDate::forUnixSeconds64(0));

  // one second after should make no difference due to truncation
  ld = LocalDate::forComponents(1970, 1, 1);
  assertEqual((int64_t) 0, ld.toUnixSeconds64());
  assertTrue(ld == LocalDate::forUnixSeconds64(0));

  ld = LocalDate::forComponents(1970, 2, 28);
  assertEqual((int64_t) 58 * 86400, ld.toUnixSeconds64());
  assertTrue(ld == LocalDate::forUnixSeconds64((int64_t) 58 * 86400));

  ld = LocalDate::forComponents(1988, 1, 1);
  assertEqual((int64_t) 6574 * 86400, ld.toUnixSeconds64());
  assertTrue(ld == LocalDate::forUnixSeconds64((int64_t) 6574 * 86400));

  // Largest date possible using Unix Seconds is 2038-01-19 03:14:07.
  ld = LocalDate::forComponents(2038, 1, 19);
  assertEqual((int64_t) 24855 * 86400, ld.toUnixSeconds64());
  assertTrue(ld == LocalDate::forUnixSeconds64((int64_t) INT32_MAX));

  // One day after the largest 32-bit dates should work in 64-bits.
  ld = LocalDate::forComponents(2038, 1, 20);
  assertEqual((int64_t) 24856 * 86400, ld.toUnixSeconds64());
  assertTrue(ld == LocalDate::forUnixSeconds64((int64_t) 24856 * 86400));

  // Verify that year 2068 works just fine with 64-bit unix seconds.
  ld = LocalDate::forComponents(2068, 1, 19);
  assertEqual((int64_t) 35812 * 86400, ld.toUnixSeconds64());
  assertTrue(ld == LocalDate::forUnixSeconds64((int64_t) 35812 * 86400));

  // Verify that year 2170 works just fine with 64-bit unix seconds.
  // 200 years = 73049 days, instead of 73048 days, because 2000 was a leap
  // year.
  ld = LocalDate::forComponents(2170, 1, 1);
  assertEqual((int64_t) 73049 * 86400, ld.toUnixSeconds64());
  assertTrue(ld == LocalDate::forUnixSeconds64((int64_t) 73049 * 86400 + 2));
}

test(LocalDateTest, compareTo) {
  LocalDate a, b;

  a = LocalDate::forComponents(2000, 1, 1);
  b = LocalDate::forComponents(2000, 1, 1);
  assertEqual(a.compareTo(b), 0);
  assertTrue(a == b);
  assertFalse(a != b);

  a = LocalDate::forComponents(2000, 1, 1);
  b = LocalDate::forComponents(2000, 1, 2);
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);

  a = LocalDate::forComponents(2000, 1, 1);
  b = LocalDate::forComponents(2000, 2, 1);
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);

  a = LocalDate::forComponents(2000, 1, 1);
  b = LocalDate::forComponents(2001, 1, 1);
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);
}

test(LocalDateTest, forDateString) {
  LocalDate ld;
  ld = LocalDate::forDateString("2000-01-01");
  assertTrue(ld == LocalDate::forComponents(2000, 1, 1));

  ld = LocalDate::forDateString("2099-02-28");
  assertTrue(ld == LocalDate::forComponents(2099, 2, 28));

  ld = LocalDate::forDateString("2127-12-31");
  assertTrue(ld == LocalDate::forComponents(2127, 12, 31));
}

test(LocalDateTest, forDateString_invalid) {
  LocalDate ld = LocalDate::forDateString("2000-01");
  assertTrue(ld.isError());
}

test(LocalDateTest, isLeapYear) {
  assertFalse(LocalDate::isLeapYear(1900));
  assertTrue(LocalDate::isLeapYear(2000));
  assertFalse(LocalDate::isLeapYear(2001));
  assertTrue(LocalDate::isLeapYear(2004));
  assertFalse(LocalDate::isLeapYear(2100));
}

test(LocalDateTest, daysInMonth) {
  assertEqual(31, LocalDate::daysInMonth(2000, 1));
  assertEqual(29, LocalDate::daysInMonth(2000, 2));
  assertEqual(31, LocalDate::daysInMonth(2000, 3));
  assertEqual(30, LocalDate::daysInMonth(2000, 4));
  assertEqual(31, LocalDate::daysInMonth(2000, 5));
  assertEqual(30, LocalDate::daysInMonth(2000, 6));
  assertEqual(31, LocalDate::daysInMonth(2000, 7));
  assertEqual(31, LocalDate::daysInMonth(2000, 8));
  assertEqual(30, LocalDate::daysInMonth(2000, 9));
  assertEqual(31, LocalDate::daysInMonth(2000, 10));
  assertEqual(30, LocalDate::daysInMonth(2000, 11));
  assertEqual(31, LocalDate::daysInMonth(2000, 12));

  assertEqual(28, LocalDate::daysInMonth(2001, 2));
  assertEqual(29, LocalDate::daysInMonth(2004, 2));
  assertEqual(28, LocalDate::daysInMonth(2100, 2));
}

//---------------------------------------------------------------------------

test(LocalDateTest, incrementOneDay) {
  LocalDate ld;

  ld = LocalDate::forComponents(2000, 2, 28);
  local_date_mutation::incrementOneDay(ld);
  assertTrue(ld == LocalDate::forComponents(2000, 2, 29));

  ld = LocalDate::forComponents(2000, 2, 29);
  local_date_mutation::incrementOneDay(ld);
  assertTrue(ld == LocalDate::forComponents(2000, 3, 1));

  ld = LocalDate::forComponents(2000, 3, 31);
  local_date_mutation::incrementOneDay(ld);
  assertTrue(ld == LocalDate::forComponents(2000, 4, 1));

  ld = LocalDate::forComponents(2000, 12, 31);
  local_date_mutation::incrementOneDay(ld);
  assertTrue(ld == LocalDate::forComponents(2001, 1, 1));

  ld = LocalDate::forComponents(2001, 2, 28);
  local_date_mutation::incrementOneDay(ld);
  assertTrue(ld == LocalDate::forComponents(2001, 3, 1));

  ld = LocalDate::forComponents(2004, 2, 28);
  local_date_mutation::incrementOneDay(ld);
  assertTrue(ld == LocalDate::forComponents(2004, 2, 29));
}

test(LocalDateTest, decrementOneDay) {
  LocalDate ld;

  ld = LocalDate::forComponents(2004, 2, 29);
  local_date_mutation::decrementOneDay(ld);
  assertTrue(ld == LocalDate::forComponents(2004, 2, 28));

  ld = LocalDate::forComponents(2001, 3, 1);
  local_date_mutation::decrementOneDay(ld);
  assertTrue(ld == LocalDate::forComponents(2001, 2, 28));

  ld = LocalDate::forComponents(2001, 1, 1);
  local_date_mutation::decrementOneDay(ld);
  assertTrue(ld == LocalDate::forComponents(2000, 12, 31));

  ld = LocalDate::forComponents(2000, 4, 1);
  local_date_mutation::decrementOneDay(ld);
  assertTrue(ld == LocalDate::forComponents(2000, 3, 31));

  ld = LocalDate::forComponents(2000, 3, 1);
  local_date_mutation::decrementOneDay(ld);
  assertTrue(ld == LocalDate::forComponents(2000, 2, 29));

  ld = LocalDate::forComponents(2000, 2, 29);
  local_date_mutation::decrementOneDay(ld);
  assertTrue(ld == LocalDate::forComponents(2000, 2, 28));
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
