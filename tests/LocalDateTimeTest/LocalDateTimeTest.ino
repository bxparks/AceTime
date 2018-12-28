#line 2 "LocalDateTimeTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time;
using namespace ace_time::common;

// --------------------------------------------------------------------------
// LocalDate
// --------------------------------------------------------------------------

test(LocalDate, accessors) {
  LocalDate ld = LocalDate::forComponents(2001, 2, 3);
  assertEqual((int16_t) 2001, ld.year());
  assertEqual(2, ld.month());
  assertEqual(3, ld.day());
}

// Verify that toEpochDays()/forEpochDays() and
// toEpochSeconds()/forEpochSeconds() support round trip conversions when when
// isError()==true.
test(LocalDate, setError) {
  LocalDate ld;

  ld = LocalDate().setError();
  assertTrue(ld.isError());
  assertEqual(LocalDate::kInvalidEpochDays, ld.toEpochDays());
  assertEqual(LocalDate::kInvalidEpochSeconds, ld.toEpochSeconds());

  ld = LocalDate::forEpochDays(LocalDate::kInvalidEpochDays);
  assertTrue(ld.isError());

  ld = LocalDate::forEpochSeconds(LocalDate::kInvalidEpochSeconds);
  assertTrue(ld.isError());
}

test(LocalDate, dayOfWeek) {
  // year 1900 (not leap year due to every 100 rule)
  assertEqual(LocalDate::kWednesday,
      LocalDate::forComponents(1900, 2, 28).dayOfWeek());
  assertEqual(LocalDate::kThursday,
      LocalDate::forComponents(1900, 3, 1).dayOfWeek());

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

test(LocalDate, toAndFromEpochDays) {
  LocalDate ld;

  // Smallest LocalDate in an 8-bit implementation
  ld = LocalDate::forComponents(1872, 1, 1);
  assertEqual((acetime_t) -46751, ld.toEpochDays());
  assertTrue(ld == LocalDate::forEpochDays(-46751));

  ld = LocalDate::forComponents(1931, 12, 14);
  assertEqual((acetime_t) -24855, ld.toEpochDays());
  assertTrue(ld == LocalDate::forEpochDays(-24855));

  ld = LocalDate::forComponents(1900, 1, 1);
  assertEqual((acetime_t) -36524, ld.toEpochDays());
  assertTrue(ld == LocalDate::forEpochDays(-36524));

  ld = LocalDate::forComponents(2000, 1, 1);
  assertEqual((acetime_t) 0, ld.toEpochDays());
  assertTrue(ld == LocalDate::forEpochDays(0));

  ld = LocalDate::forComponents(2000, 2, 29);
  assertEqual((acetime_t) 59, ld.toEpochDays());
  assertTrue(ld == LocalDate::forEpochDays(59));

  ld = LocalDate::forComponents(2018, 1, 1);
  assertEqual((acetime_t) 6575, ld.toEpochDays());
  assertTrue(ld == LocalDate::forEpochDays(6575));

  ld = LocalDate::forComponents(2099, 12, 31);
  assertEqual((acetime_t) 36524, ld.toEpochDays());
  assertTrue(ld == LocalDate::forEpochDays(36524));

  // Smallest LocalDate in an 8-bit implementation
  ld = LocalDate::forComponents(2127, 12, 31);
  assertEqual((acetime_t) 46750, ld.toEpochDays());
  assertTrue(ld == LocalDate::forEpochDays(46750));
}

test(LocalDate, toAndFromEpochSeconds) {
  LocalDate ld;

  // The smallest whole day that can be represented with an int32_t is
  // 1931-12-14.
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
  // 2068-01-19 03:14:06 (largest date at INT32_MAX - 1)
  assertTrue(ld == LocalDate::forEpochSeconds(
      (acetime_t) 24855 * 86400 + 11646));
}

test(LocalDate, compareTo) {
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

test(LocalDate, forDateString) {
  LocalDate ld;
  ld = LocalDate::forDateString("2000-01-01");
  assertTrue(ld == LocalDate::forComponents(2000, 1, 1));

  ld = LocalDate::forDateString("2099-02-28");
  assertTrue(ld == LocalDate::forComponents(2099, 2, 28));

  ld = LocalDate::forDateString("2127-12-31");
  assertTrue(ld == LocalDate::forComponents(2127, 12, 31));
}

test(LocalDate, forDateString_invalid) {
  LocalDate ld = LocalDate::forDateString("2000-01");
  assertTrue(ld.isError());
}

test(LocalDate, isLeapYear) {
  assertFalse(LocalDate::isLeapYear(1900));
  assertTrue(LocalDate::isLeapYear(2000));
  assertFalse(LocalDate::isLeapYear(2001));
  assertTrue(LocalDate::isLeapYear(2004));
  assertFalse(LocalDate::isLeapYear(2100));
}

test(LocalDate, daysInMonth) {
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

test(LocalDate, incrementYear) {
  LocalDate ld = LocalDate::forComponents(2000, 1, 1);
  ld.incrementYear();
  assertEqual((int16_t) 2001, ld.year());

  ld.year(2099);
  ld.incrementYear();
  assertEqual((int16_t) 2000, ld.year());
}

test(LocalDate, incrementMonth) {
  LocalDate ld = LocalDate::forComponents(2000, 1, 1);
  ld.incrementMonth();
  assertEqual(2, ld.month());

  ld.month(12);
  ld.incrementMonth();
  assertEqual(1, ld.month());
}

test(LocalDate, incrementDay) {
  LocalDate ld = LocalDate::forComponents(2000, 1, 1);
  ld.incrementDay();
  assertEqual(2, ld.day());

  ld.day(31);
  ld.incrementDay();
  assertEqual(1, ld.day());
}

// --------------------------------------------------------------------------
// LocalTime
// --------------------------------------------------------------------------

test(LocalTime, accessors) {
  LocalTime lt = LocalTime::forComponents(1, 2, 3);
  assertEqual(1, lt.hour());
  assertEqual(2, lt.minute());
  assertEqual(3, lt.second());
}

test(LocalTime, setError) {
  LocalTime lt = LocalTime().setError();
  assertTrue(lt.isError());
  assertEqual(LocalTime::kInvalidSeconds, lt.toSeconds());

  lt = LocalTime::forSeconds(LocalTime::kInvalidSeconds);
  assertTrue(lt.isError());
}

test(LocalTime, toAndFromSeconds) {
  LocalTime lt;

  lt = LocalTime::forSeconds(0);
  assertTrue(lt == LocalTime::forComponents(0, 0, 0));
  assertEqual((acetime_t) 0, lt.toSeconds());

  lt = LocalTime::forSeconds(3662);
  assertTrue(lt == LocalTime::forComponents(1, 1, 2));
  assertEqual((acetime_t) 3662, lt.toSeconds());

  lt = LocalTime::forSeconds(86399);
  assertTrue(lt == LocalTime::forComponents(23, 59, 59));
  assertEqual((acetime_t) 86399, lt.toSeconds());
}

test(LocalTime, compareTo) {
  LocalTime a, b;

  a = LocalTime::forComponents(0, 1, 1);
  b = LocalTime::forComponents(0, 1, 1);
  assertEqual(a.compareTo(b), 0);
  assertTrue(a == b);
  assertFalse(a != b);

  a = LocalTime::forComponents(0, 1, 1);
  b = LocalTime::forComponents(0, 1, 2);
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);

  a = LocalTime::forComponents(0, 1, 1);
  b = LocalTime::forComponents(0, 2, 1);
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);

  a = LocalTime::forComponents(0, 1, 1);
  b = LocalTime::forComponents(1, 1, 1);
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);
}

test(LocalTime, forTimeString) {
  LocalTime lt;
  lt = LocalTime::forTimeString("00:00:00");
  assertTrue(lt == LocalTime::forComponents(0, 0, 0));

  lt = LocalTime::forTimeString("01:02:03");
  assertTrue(lt == LocalTime::forComponents(1, 2, 3));
}

test(LocalTime, fortimeString_invalid) {
  LocalTime lt = LocalTime::forTimeString("01:02");
  assertTrue(lt.isError());
}

test(LocalTime, incrementHour) {
  LocalTime lt = LocalTime::forComponents(0, 0, 0);
  lt.incrementHour();
  assertEqual(1, lt.hour());

  lt.hour(23);
  lt.incrementHour();
  assertEqual(0, lt.hour());
}

test(LocalTime, incrementMinute) {
  LocalTime lt = LocalTime::forComponents(0, 0, 0);
  lt.incrementMinute();
  assertEqual(1, lt.minute());

  lt.minute(59);
  lt.incrementMinute();
  assertEqual(0, lt.minute());
}

// --------------------------------------------------------------------------

void setup() {
  delay(1000); // wait for stability on some boards to prevent garbage Serial
  Serial.begin(115200); // ESP8266 default of 74880 not supported on Linux
  while(!Serial); // for the Arduino Leonardo/Micro only
}

void loop() {
  TestRunner::run();
}
