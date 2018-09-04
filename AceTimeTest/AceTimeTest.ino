#include <AceTime.h>
#include <AUnit.h>

using namespace aunit;
using namespace ace_time;

// --------------------------------------------------------------------------
// DateTime
// --------------------------------------------------------------------------

test(daysAndSecondsSinceEpochAt2000_01_01) {
  DateTime dt{0, 1, 1, 0, 0, 0, TimeZone(0)}; // 2000-01-01 00:00:00Z

  uint32_t daysSinceEpoch = dt.toDaysSinceEpoch();
  assertEqual((uint32_t) 0, daysSinceEpoch);

  uint32_t secondsSinceEpoch = dt.toSecondsSinceEpoch();
  assertEqual((uint32_t) 0, secondsSinceEpoch);

  assertEqual(7, dt.dayOfWeek());
}

// 2000-02-29 was a leap year, due to the every 400 year rule
// 2100-02-29 is *not* a leap year, due to the every 100 year rule
test(daysAndSecondsSinceEpochAt2000_02_29) {
  DateTime dt{0, 2, 29, 0, 0, 0, TimeZone(0)}; // 2000-02-29 00:00:00Z

  uint32_t daysSinceEpoch = dt.toDaysSinceEpoch();
  assertEqual((uint32_t) 59, daysSinceEpoch);

  uint32_t secondsSinceEpoch = dt.toSecondsSinceEpoch();
  assertEqual((uint32_t) 86400 * 59, secondsSinceEpoch);

  assertEqual(3, dt.dayOfWeek());
}

test(daysAndSecondsSinceEpochAt2000_01_02) {
  DateTime dt{0, 1, 2, 0, 0, 0, TimeZone(0)}; // 2000-01-02 00:00:00Z

  uint32_t daysSinceEpoch = dt.toDaysSinceEpoch();
  assertEqual((uint32_t) 1, daysSinceEpoch);

  uint32_t secondsSinceEpoch = dt.toSecondsSinceEpoch();
  assertEqual((uint32_t) 86400, secondsSinceEpoch);

  assertEqual(1, dt.dayOfWeek());
}

test(daysAndSecondsSinceEpochAt2018_01_01) {
  DateTime dt{18, 1, 1, 0, 0, 0, TimeZone(0)}; // 2018-01-01 00:00:00Z

  uint32_t daysSinceEpoch = dt.toDaysSinceEpoch();
  assertEqual((uint32_t) 6575, daysSinceEpoch);

  uint32_t secondsSinceEpoch = dt.toSecondsSinceEpoch();
  assertEqual(6575 * (uint32_t) 86400, secondsSinceEpoch);

  assertEqual(2, dt.dayOfWeek());
}

test(daysAndSecondsSinceEpochAt2018_01_01WithTimeZone) {
  DateTime dt{18, 1, 1, 0, 0, 0, TimeZone(1)}; // 2018-01-01 00:00:00+00:15

  uint32_t daysSinceEpoch = dt.toDaysSinceEpoch();
  assertEqual((uint32_t) 6574, daysSinceEpoch);

  uint32_t secondsSinceEpoch = dt.toSecondsSinceEpoch();
  assertEqual(6575 * (uint32_t) 86400 - 15*60, secondsSinceEpoch);

  assertEqual(2, dt.dayOfWeek()); // 2018-01-01 is a Monday
}

test(daysAndSecondsSinceEpochAt2049_12_31) {
  DateTime dt{49, 12, 31, 23, 59, 59, TimeZone(0)}; // 2049-12-31 23:59:59Z

  uint32_t daysSinceEpoch = dt.toDaysSinceEpoch();
  assertEqual((uint32_t) 18262, daysSinceEpoch);

  uint32_t secondsSinceEpoch = dt.toSecondsSinceEpoch();
  assertEqual(18263 * (uint32_t) 86400 - 1, secondsSinceEpoch);

  assertEqual(6, dt.dayOfWeek());
}

test(daysAndSecondsSinceEpochAt2049_12_31WithTimeZone) {
  DateTime dt{49, 12, 31, 23, 59, 59, TimeZone(-1)}; // 2049-12-31 23:59:59-00:15

  uint32_t daysSinceEpoch = dt.toDaysSinceEpoch();
  assertEqual((uint32_t) 18263, daysSinceEpoch);

  uint32_t secondsSinceEpoch = dt.toSecondsSinceEpoch();
  assertEqual(18263 * (uint32_t) 86400 + 15*60 - 1, secondsSinceEpoch);

  assertEqual(6, dt.dayOfWeek()); // 2049-12-31 is a Friday
}

test(daysAndSsecondsSinceEpochAt2050_01_01) {
  DateTime dt{50, 1, 1, 0, 0, 0, TimeZone(0)}; // 2050-01-01 00:00:00Z

  uint32_t daysSinceEpoch = dt.toDaysSinceEpoch();
  assertEqual((uint32_t) 18263, daysSinceEpoch);

  uint32_t secondsSinceEpoch = dt.toSecondsSinceEpoch();
  assertEqual(18263 * (uint32_t) 86400, secondsSinceEpoch);

  assertEqual(7, dt.dayOfWeek()); // Saturday
}

test(daysAndSsecondsSinceEpochAt2099_12_31) {
  DateTime dt{99, 12, 31, 23, 59, 59, TimeZone(0)}; // 2099-12-31 23:59:59Z

  uint32_t daysSinceEpoch = dt.toDaysSinceEpoch();
  assertEqual((uint32_t) 36524, daysSinceEpoch);

  uint32_t secondsSinceEpoch = dt.toSecondsSinceEpoch();
  assertEqual(36525 * (uint32_t) 86400 - 1, secondsSinceEpoch);

  assertEqual(5, dt.dayOfWeek()); // Thursday
}

test(unixSeconds) {
  DateTime dt{0, 1, 1, 0, 0, 0, TimeZone(0)}; // 2000-01-01 00:00:00Z
  assertEqual((uint32_t) 946684800, dt.toUnixSeconds());

  dt = DateTime{18, 1, 1, 0, 0, 0, TimeZone(0)}; // 2018-01-01 00:00:00Z
  assertEqual((uint32_t) 1514764800, dt.toUnixSeconds());

  dt = DateTime{18, 8, 30, 6, 45, 1, TimeZone(-28)}; // 2018-08-30T06:45:01-07:00
  assertEqual((uint32_t) 1535636701, dt.toUnixSeconds());

  dt = DateTime{38, 1, 1, 0, 0, 0, TimeZone(0)}; // 2038-01-01 00:00:00Z
  assertEqual((uint32_t) 2145916800, dt.toUnixSeconds());

  // 2099-12-31 23:59:59-16:00
  dt = DateTime{99, 12, 31, 23, 59, 59, TimeZone(-64)};
  assertEqual((uint32_t) 4102502399, dt.toUnixSeconds());
}

test(constructFromSecondsSinceEpochAt2049_12_31) {
  DateTime dt(18263 * (int32_t) 86400 - 1); // 2049-12-31 23:59:59Z

  assertEqual(49, dt.year());
  assertEqual(12, dt.month());
  assertEqual(31, dt.day());
  assertEqual(23, dt.hour());
  assertEqual(59, dt.minute());
  assertEqual(59, dt.second());
  assertEqual(6, dt.dayOfWeek());

  // 2049-12-31 15:59:59-08:00
  TimeZone tz(-32); // UTC-08:00
  dt = DateTime(18263 * (int32_t) 86400 - 1, tz);
  assertEqual(49, dt.year());
  assertEqual(12, dt.month());
  assertEqual(31, dt.day());
  assertEqual(15, dt.hour());
  assertEqual(59, dt.minute());
  assertEqual(59, dt.second());
  assertEqual(6, dt.dayOfWeek());
}

test(convertToTimeZone) {
  DateTime a{18, 1, 1, 12, 0, 0, TimeZone(0)};
  DateTime b = a.convertToTimeZone(TimeZone(-28)); // UTC-07:00

  assertEqual(18, b.year());
  assertEqual(1, b.month());
  assertEqual(1, b.day());
  assertEqual(5, b.hour());
  assertEqual(0, b.minute());
  assertEqual(0, b.second());
  assertEqual(-28, b.timeZone().tzCode());
}

test(dateTimeCompareAndEquals) {
  DateTime a{18, 1, 1, 12, 0, 0, TimeZone(0)};
  DateTime b{18, 1, 1, 12, 0, 0, TimeZone(0)};
  assertEqual(a.compareTo(b), 0);
  assertTrue(a == b);
  assertFalse(a != b);

  a = DateTime{18, 1, 1, 12, 0, 0, TimeZone(0)};
  b = DateTime{18, 1, 1, 12, 0, 1, TimeZone(0)};
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);

  a = DateTime{18, 1, 1, 12, 0, 0, TimeZone(0)};
  b = DateTime{18, 1, 1, 12, 1, 0, TimeZone(0)};
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);

  a = DateTime{18, 1, 1, 11, 0, 0, TimeZone(0)};
  b = DateTime{18, 1, 1, 12, 0, 0, TimeZone(1)};
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);

  a = DateTime{18, 1, 1, 12, 0, 0, TimeZone(0)};
  b = DateTime{18, 1, 2, 12, 0, 0, TimeZone(0)};
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);

  a = DateTime{18, 1, 1, 12, 0, 0, TimeZone(0)};
  b = DateTime{18, 2, 1, 12, 0, 0, TimeZone(0)};
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);

  a = DateTime{18, 1, 1, 12, 0, 0, TimeZone(0)};
  b = DateTime{19, 1, 1, 12, 0, 0, TimeZone(0)};
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);

  a = DateTime{18, 1, 1, 12, 0, 0, TimeZone(4)}; // 2018-1-1 12:00:00+01:00
  b = DateTime{18, 1, 1, 12, 0, 0, TimeZone(-32)}; // 2018-1-1 12:00:00-08:00
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);
}

test(calculateAndCacheDayOfWeek) {
  DateTime dt{18, 1, 1, 0, 0, 0, TimeZone(0)}; // 2018-01-01 00:00:00Z
  assertEqual(2, dt.dayOfWeek()); // Monday

  dt.hour(23); // no change to dayOfWeek
  assertEqual(2, dt.dayOfWeek());

  dt.minute(40); // no change to dayOfWeek
  assertEqual(2, dt.dayOfWeek());

  dt.second(3); // no change to dayOfWeek
  assertEqual(2, dt.dayOfWeek());

  dt.timeZone(TimeZone(3)); // no change to dayOfWeek
  assertEqual(2, dt.dayOfWeek());

  dt.day(2); // changes dayOfWeek
  assertEqual(3, dt.dayOfWeek());

  dt.month(2); // changes dayOfWeek
  assertEqual(6, dt.dayOfWeek()); // Friday

  dt.year(19); // changes dayOfWeek
  assertEqual(7, dt.dayOfWeek());
}

test(dateTimeIsError) {
  DateTime dt(18, 1, 1, 0, 0, 0); // 2018-01-01 00:00:00Z
  assertFalse(dt.isError());

  dt = DateTime(18, 0, 1, 0, 0, 0);
  assertTrue(dt.isError());

  dt = DateTime(18, 255, 1, 0, 0, 0);
  assertTrue(dt.isError());

  dt = DateTime(18, 1, 0, 0, 0, 0);
  assertTrue(dt.isError());

  dt = DateTime(18, 1, 255, 0, 0, 0);
  assertTrue(dt.isError());

  dt = DateTime(18, 1, 1, 255, 0, 0);
  assertTrue(dt.isError());

  dt = DateTime(18, 1, 1, 0, 255, 0);
  assertTrue(dt.isError());

  dt = DateTime(18, 1, 1, 0, 0, 255);
  assertTrue(dt.isError());
}

test(dateTimeFromString_errors) {
  // empty string, too short
  DateTime dt("");
  assertTrue(dt.isError());

  // not enough components
  dt = DateTime(F("2018-08-31"));
  assertTrue(dt.isError());

  // too long
  dt = DateTime(F("2018-08-31T13:48:01-07:00X"));
  assertTrue(dt.isError());

  // too short
  dt = DateTime(F("2018-08-31T13:48:01-07:0"));
  assertTrue(dt.isError());

  // missing UTC
  dt = DateTime(F("2018-08-31T13:48:01"));
  assertTrue(dt.isError());

  // parser cares about UTC+/- offset
  dt = DateTime(F("2018-08-31 13:48:01&07:00"));
  assertTrue(dt.isError());
}

test(dateTimFromString) {
  // exact ISO8601 format
  DateTime dt = DateTime(F("2018-08-31T13:48:01-07:00"));
  assertFalse(dt.isError());
  assertEqual(18, dt.year());
  assertEqual(8, dt.month());
  assertEqual(31, dt.day());
  assertEqual(13, dt.hour());
  assertEqual(48, dt.minute());
  assertEqual(1, dt.second());
  assertEqual(-28, dt.timeZone().tzCode());
  assertEqual(6, dt.dayOfWeek());

  // parser does not care about most separators, this may change in the future
  dt = DateTime(F("2018/08/31 13#48#01+07#00"));
  assertFalse(dt.isError());
  assertEqual(18, dt.year());
  assertEqual(8, dt.month());
  assertEqual(31, dt.day());
  assertEqual(13, dt.hour());
  assertEqual(48, dt.minute());
  assertEqual(1, dt.second());
  assertEqual(28, dt.timeZone().tzCode());
  assertEqual(6, dt.dayOfWeek());
}

// --------------------------------------------------------------------------
// TimePeriod
// --------------------------------------------------------------------------

test(timePeriodFromComponents) {
  TimePeriod t(0, 16, 40, 1);
  assertEqual(t.toSeconds(), (int32_t) 1000);

  TimePeriod u(0, 16, 40, -1);
  assertEqual(u.toSeconds(), (int32_t) -1000);
}

test(timePeriodFromSeconds) {
  TimePeriod t(1000);
  assertEqual(0, t.hour());
  assertEqual(16, t.minute());
  assertEqual(40, t.second());
  assertEqual(1, t.sign());
  assertEqual((int32_t) 1000, t.toSeconds());

  TimePeriod large((int32_t) 100000);
  assertEqual(27, large.hour());
  assertEqual(46, large.minute());
  assertEqual(40, large.second());
  assertEqual(1, t.sign());
  assertEqual((int32_t) 100000, large.toSeconds());
}

test(timePeriodCompareAndEquals) {
  TimePeriod a(3, 2, 1, 1);
  TimePeriod b(3, 2, 1, 1);
  assertEqual(a.compareTo(b), 0);
  assertTrue(a == b);
  assertFalse(a != b);

  a = TimePeriod(3, 2, 1, 1);
  b = TimePeriod(3, 2, 2, 1);
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);

  a = TimePeriod(3, 2, 1, 1);
  b = TimePeriod(3, 3, 1, 1);
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);

  a = TimePeriod(3, 2, 1, 1);
  b = TimePeriod(4, 2, 1, 1);
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);

  a = TimePeriod(3, 2, 1, -1);
  b = TimePeriod(3, 2, 1, 1);
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);
}

test(timePeriodIncrementHour) {
  TimePeriod a(3, 2, 1, 1);
  a.incrementHour();
  TimePeriod expected(4, 2, 1, 1);
  assertTrue(expected == a);

  a = TimePeriod(23, 2, 1, -1);
  a.incrementHour();
  expected = TimePeriod(0, 2, 1, -1);
  assertTrue(expected == a);
}

test(timePeriodIncrementMinute) {
  TimePeriod a(3, 2, 1, 1);
  a.incrementMinute();
  TimePeriod expected(3, 3, 1, 1);
  assertTrue(expected == a);

  a = TimePeriod(3, 59, 1, 1);
  a.incrementMinute();
  expected = TimePeriod(3, 0, 1, 1);
  assertTrue(expected == a);
}

test(timePeriodNegate) {
  TimePeriod a(3, 2, 1, 1);
  assertEqual((int8_t) 1, a.sign());
  a.negate();
  assertEqual((int8_t) -1, a.sign());
}

// --------------------------------------------------------------------------
// TimeZone
// --------------------------------------------------------------------------

test(timeZone) {
  TimeZone tz(-1);
  assertEqual((int16_t) -15, tz.toMinutes());
  assertEqual((int32_t) -900, tz.toSeconds());

  tz = TimeZone(1);
  assertEqual((int16_t) 15, tz.toMinutes());
  assertEqual((int32_t) 900, tz.toSeconds());
}

test(timeZoneIncrementHour) {
  TimeZone tz(-1);
  tz.incrementHour();
  assertEqual((int8_t) 3, tz.tzCode());

  tz = TimeZone(63);
  tz.incrementHour();
  assertEqual((int8_t) -61, tz.tzCode());

  tz = TimeZone(60);
  tz.incrementHour();
  assertEqual((int8_t) -64, tz.tzCode());
}

test(timeZoneIncrement15Minutes) {
  TimeZone tz(3);

  tz.increment15Minutes();
  assertEqual((int8_t) 0, tz.tzCode());

  tz.increment15Minutes();
  assertEqual((int8_t) 1, tz.tzCode());

  tz.increment15Minutes();
  assertEqual((int8_t) 2, tz.tzCode());

  tz.increment15Minutes();
  assertEqual((int8_t) 3, tz.tzCode());

  tz = TimeZone(-4);
  tz.increment15Minutes();
  assertEqual((int8_t) -5, tz.tzCode());

  tz.increment15Minutes();
  assertEqual((int8_t) -6, tz.tzCode());

  tz.increment15Minutes();
  assertEqual((int8_t) -7, tz.tzCode());

  tz.increment15Minutes();
  assertEqual((int8_t) -4, tz.tzCode());
}

test(timeZoneToHourMinute) {
  TimeZone tz(-29);
  uint8_t hour, minute;
  tz.toHourMinute(hour, minute);
  assertEqual(7, hour);
  assertEqual(15, minute);
}

// --------------------------------------------------------------------------
// DateStrings
// --------------------------------------------------------------------------

test(monthString) {
  DateStrings ds;

  assertEqual("Error", ds.monthLongString(0));
  assertEqual("January", ds.monthLongString(1));
  assertEqual("February", ds.monthLongString(2));
  assertEqual("March", ds.monthLongString(3));
  assertEqual("April", ds.monthLongString(4));
  assertEqual("May", ds.monthLongString(5));
  assertEqual("June", ds.monthLongString(6));
  assertEqual("July", ds.monthLongString(7));
  assertEqual("August", ds.monthLongString(8));
  assertEqual("September", ds.monthLongString(9));
  assertEqual("October", ds.monthLongString(10));
  assertEqual("November", ds.monthLongString(11));
  assertEqual("December", ds.monthLongString(12));
  assertEqual("Error", ds.monthLongString(13));

  assertEqual("Err", ds.monthShortString(0));
  assertEqual("Jan", ds.monthShortString(1));
  assertEqual("Feb", ds.monthShortString(2));
  assertEqual("Mar", ds.monthShortString(3));
  assertEqual("Apr", ds.monthShortString(4));
  assertEqual("May", ds.monthShortString(5));
  assertEqual("Jun", ds.monthShortString(6));
  assertEqual("Jul", ds.monthShortString(7));
  assertEqual("Aug", ds.monthShortString(8));
  assertEqual("Sep", ds.monthShortString(9));
  assertEqual("Oct", ds.monthShortString(10));
  assertEqual("Nov", ds.monthShortString(11));
  assertEqual("Dec", ds.monthShortString(12));
  assertEqual("Err", ds.monthShortString(13));

  uint8_t maxLength = 0;
  for (uint8_t month = 0; month <= 12; month++) {
    const char* monthString = ds.monthLongString(month);
    uint8_t length = strlen(monthString);
    if (length > maxLength) { maxLength = length; }
  }
  assertLessOrEqual(maxLength, DateStrings::kBufferSize - 1);
}

test(weekDayStrings) {
  DateStrings ds;

  assertEqual("Error", ds.weekDayLongString(0));
  assertEqual("Sunday", ds.weekDayLongString(1));
  assertEqual("Monday", ds.weekDayLongString(2));
  assertEqual("Tuesday", ds.weekDayLongString(3));
  assertEqual("Wednesday", ds.weekDayLongString(4));
  assertEqual("Thursday", ds.weekDayLongString(5));
  assertEqual("Friday", ds.weekDayLongString(6));
  assertEqual("Saturday", ds.weekDayLongString(7));
  assertEqual("Error", ds.weekDayLongString(8));

  assertEqual("Err", ds.weekDayShortString(0));
  assertEqual("Sun", ds.weekDayShortString(1));
  assertEqual("Mon", ds.weekDayShortString(2));
  assertEqual("Tue", ds.weekDayShortString(3));
  assertEqual("Wed", ds.weekDayShortString(4));
  assertEqual("Thu", ds.weekDayShortString(5));
  assertEqual("Fri", ds.weekDayShortString(6));
  assertEqual("Sat", ds.weekDayShortString(7));
  assertEqual("Err", ds.weekDayShortString(8));

  uint8_t maxLength = 0;
  for (uint8_t weekDay = 0; weekDay <= 7; weekDay++) {
    const char* weekDayString = ds.weekDayLongString(weekDay);
    uint8_t length = strlen(weekDayString);
    if (length > maxLength) { maxLength = length; }
  }
  assertLessOrEqual(maxLength, DateStrings::kBufferSize - 1);
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
