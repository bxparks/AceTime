#line 2 "LocalDateTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time;

//---------------------------------------------------------------------------
// LocalDate
//---------------------------------------------------------------------------

test(LocalDateTest, isError_year_out_of_range) {
  assertTrue(LocalDate::forComponents(0, 1, 1).isError());
  assertTrue(LocalDate::forComponents(1872, 1, 1).isError());
  assertTrue(LocalDate::forComponents(2128, 1, 1).isError());
  assertTrue(LocalDate::forComponents(9999, 1, 1).isError());
}

test(LocalDateTest, isError_month_out_of_range) {
  assertTrue(LocalDate::forComponents(2000, 0, 1).isError());
  assertTrue(LocalDate::forComponents(2000, 13, 1).isError());
}

test(LocalDateTest, isError_day_out_of_range) {
  assertTrue(LocalDate::forComponents(2000, 1, 0).isError());
  assertTrue(LocalDate::forComponents(2000, 1, 32).isError());
}

test(LocalDateTest, isError_valid_at_boundaries) {
  assertFalse(LocalDate::forComponents(1873, 1, 1).isError());
  assertFalse(LocalDate::forComponents(2127, 12, 31).isError());
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
test(LocalDateTest, forError) {
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

  // Smallest LocalDate in an 8-bit implementation
  ld = LocalDate::forComponents(1873, 1, 1);
  assertEqual((acetime_t) -46385, ld.toEpochDays());
  assertTrue(ld == LocalDate::forEpochDays(-46385));

  ld = LocalDate::forComponents(1900, 1, 1);
  assertEqual((acetime_t) -36524, ld.toEpochDays());
  assertTrue(ld == LocalDate::forEpochDays(-36524));

  // Smallest date using int32_t from AceTime epoch
  ld = LocalDate::forComponents(1931, 12, 14);
  assertEqual((acetime_t) -24855, ld.toEpochDays());
  assertTrue(ld == LocalDate::forEpochDays(-24855));

  // AceTime Epoch
  ld = LocalDate::forComponents(2000, 1, 1);
  assertEqual((acetime_t) 0, ld.toEpochDays());
  assertTrue(ld == LocalDate::forEpochDays(0));

  ld = LocalDate::forComponents(2000, 2, 29);
  assertEqual((acetime_t) 59, ld.toEpochDays());
  assertTrue(ld == LocalDate::forEpochDays(59));

  ld = LocalDate::forComponents(2018, 1, 1);
  assertEqual((acetime_t) 6575, ld.toEpochDays());
  assertTrue(ld == LocalDate::forEpochDays(6575));

  // Largest date using int32_t from AceTimeEpoch
  ld = LocalDate::forComponents(2068, 1, 19);
  assertEqual((acetime_t) 24855, ld.toEpochDays());
  assertTrue(ld == LocalDate::forEpochDays(24855));

  // Largest LocalDate in an 8-bit implementation
  ld = LocalDate::forComponents(2127, 12, 31);
  assertEqual((acetime_t) 46750, ld.toEpochDays());
  assertTrue(ld == LocalDate::forEpochDays(46750));
}

// Same as toAndFromEpochDays, shifted 30 years
test(LocalDateTest, toAndFromUnixDays) {
  LocalDate ld;

  // Smallest LocalDate in an 8-bit implementation
  ld = LocalDate::forComponents(1873, 1, 1);
  assertEqual((acetime_t) -35428, ld.toUnixDays());
  assertTrue(ld == LocalDate::forUnixDays(-35428));

  // Smallest date using int32_t from Unix epoch
  ld = LocalDate::forComponents(1901, 12, 14);
  assertEqual((acetime_t) -24855, ld.toUnixDays());
  assertTrue(ld == LocalDate::forUnixDays(-24855));

  // Unix Epoch
  ld = LocalDate::forComponents(1970, 1, 1);
  assertEqual((acetime_t) 0, ld.toUnixDays());
  assertTrue(ld == LocalDate::forUnixDays(0));

  // 1970 is not a leap year, whereas 2000 is a leap year
  ld = LocalDate::forComponents(1970, 2, 28);
  assertEqual((acetime_t) 58, ld.toUnixDays());
  assertTrue(ld == LocalDate::forUnixDays(58));

  ld = LocalDate::forComponents(1988, 1, 1);
  assertEqual((acetime_t) 6574, ld.toUnixDays());
  assertTrue(ld == LocalDate::forUnixDays(6574));

  // Largest date using int32_t from Unix epoch
  ld = LocalDate::forComponents(2038, 1, 19);
  assertEqual((acetime_t) 24855, ld.toUnixDays());
  assertTrue(ld == LocalDate::forUnixDays(24855));

  // Largest LocalDate in an 8-bit implementation
  ld = LocalDate::forComponents(2127, 12, 31);
  assertEqual((acetime_t) 57707, ld.toUnixDays());
  assertTrue(ld == LocalDate::forUnixDays(57707));
}

test(LocalDateTest, toAndFromEpochSeconds) {
  LocalDate ld;

  // The smallest whole day that can be represented with an int32_t from
  // AceTime Epoch is 1931-12-14.
  ld = LocalDate::forComponents(1931, 12, 14);
  assertEqual((acetime_t) -24855 * 86400, ld.toEpochSeconds());
  assertTrue(ld == LocalDate::forEpochSeconds((acetime_t) -24855 * 86400 + 60));

  // Smallest date with an int32_t is 1931-12-13 20:45:52. The
  // forEpochSeconds() will correctly truncate the partial day *down* towards
  // the to the nearest whole day.
  ld = LocalDate::forComponents(1931, 12, 13);
  assertTrue(ld == LocalDate::forEpochSeconds(INT32_MIN + 1));

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

test(LocalDateTest, toAndFromUnixSeconds) {
  LocalDate ld;

  // The smallest whole day that can be represented with an int32_t from AceTime
  // epoch is 1931-12-14, can't do better with unixSeconds since it uses
  // the Acetime seconds internally.
  ld = LocalDate::forComponents(1931, 12, 14);
  assertEqual((acetime_t) -13898 * 86400, ld.toUnixSeconds());
  assertTrue(ld == LocalDate::forUnixSeconds((acetime_t) -13898 * 86400 + 60));

  ld = LocalDate::forComponents(1970, 1, 1);
  assertEqual((acetime_t) 0, ld.toUnixSeconds());
  assertTrue(ld == LocalDate::forUnixSeconds(0));

  ld = LocalDate::forComponents(1970, 2, 28);
  assertEqual((acetime_t) 58 * 86400, ld.toUnixSeconds());
  assertTrue(ld == LocalDate::forUnixSeconds((acetime_t) 58 * 86400 + 1));

  ld = LocalDate::forComponents(1988, 1, 1);
  assertEqual((acetime_t) 6574 * 86400, ld.toUnixSeconds());
  assertTrue(ld == LocalDate::forUnixSeconds((acetime_t) 6574 * 86400 + 2));

  // Largest date possible using Unix Seconds is 2038-01-19 03:14:07.
  ld = LocalDate::forComponents(2038, 1, 19);
  assertEqual((acetime_t) 24855 * 86400, ld.toUnixSeconds());
  assertTrue(ld == LocalDate::forUnixSeconds((acetime_t) INT32_MAX));
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

test(LocalDateTest, incrementOneDay_error) {
  auto ld = LocalDate::forComponents(2127, 12, 31);
  local_date_mutation::incrementOneDay(ld);
  assertTrue(ld.isError());
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

test(LocalDateTest, decrementOneDay_error) {
  auto ld = LocalDate::forComponents(1873, 1, 1);
  local_date_mutation::decrementOneDay(ld);
  assertTrue(ld.isError());
}

//---------------------------------------------------------------------------

void setup() {
#if ! defined(UNIX_HOST_DUINO)
  delay(1000); // wait to prevent garbage on SERIAL_PORT_MONITOR
#endif
  SERIAL_PORT_MONITOR.begin(115200);
  while (!SERIAL_PORT_MONITOR); // Leonardo/Micro
}

void loop() {
  TestRunner::run();
}
