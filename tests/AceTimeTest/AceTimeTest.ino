#line 2 "AceTimeTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time;
using namespace ace_time::common;

// --------------------------------------------------------------------------
// OffsetDateTime
// --------------------------------------------------------------------------

test(OffsetDateTimeTest, accessors) {
  OffsetDateTime dt = OffsetDateTime::forComponents(1, 2, 3, 4, 5, 6);
  assertEqual((uint16_t) 2001, dt.yearFull());
  assertEqual(1, dt.year());
  assertEqual(2, dt.month());
  assertEqual(3, dt.day());
  assertEqual(4, dt.hour());
  assertEqual(5, dt.minute());
  assertEqual(6, dt.second());
  assertEqual(0, dt.zoneOffset().toOffsetCode());
}

test(OffsetDateTimeTest, invalidSeconds) {
  OffsetDateTime dt = OffsetDateTime::forEpochSeconds(
      OffsetDateTime::kInvalidEpochSeconds);
  assertTrue(dt.isError());
  assertEqual(OffsetDateTime::kInvalidEpochSeconds, dt.toEpochSeconds());
  assertEqual(OffsetDateTime::kInvalidEpochDays, dt.toEpochDays());
}

test(OffsetDateTimeTest, setError) {
  OffsetDateTime dt = OffsetDateTime().setError();
  assertTrue(dt.isError());
}

test(OffsetDateTimeTest, isError) {
  // 2018-01-01 00:00:00Z
  OffsetDateTime dt = OffsetDateTime::forComponents(18, 1, 1, 0, 0, 0);
  assertFalse(dt.isError());

  dt = OffsetDateTime::forComponents(18, 0, 1, 0, 0, 0);
  assertTrue(dt.isError());

  dt = OffsetDateTime::forComponents(18, 255, 1, 0, 0, 0);
  assertTrue(dt.isError());

  dt = OffsetDateTime::forComponents(18, 1, 0, 0, 0, 0);
  assertTrue(dt.isError());

  dt = OffsetDateTime::forComponents(18, 1, 255, 0, 0, 0);
  assertTrue(dt.isError());

  dt = OffsetDateTime::forComponents(18, 1, 1, 255, 0, 0);
  assertTrue(dt.isError());

  dt = OffsetDateTime::forComponents(18, 1, 1, 0, 255, 0);
  assertTrue(dt.isError());

  dt = OffsetDateTime::forComponents(18, 1, 1, 0, 0, 255);
  assertTrue(dt.isError());
}

test(OffsetDateTimeTest, toAndFromEpochSeconds) {
  OffsetDateTime dt;

  // 2000-01-01 00:00:00Z Saturday
  dt = OffsetDateTime::forComponents(0, 1, 1, 0, 0, 0);
  assertEqual((uint32_t) 0, dt.toEpochDays());
  assertEqual((uint32_t) 0, dt.toEpochSeconds());
  assertEqual(LocalDate::kSaturday, dt.dayOfWeek());

  // 2000-02-29 00:00:00Z Tuesday
  dt = OffsetDateTime::forComponents(0, 2, 29, 0, 0, 0);
  assertEqual((uint32_t) 59, dt.toEpochDays());
  assertEqual((uint32_t) 86400 * 59, dt.toEpochSeconds());
  assertEqual(LocalDate::kTuesday, dt.dayOfWeek());

  // 2000-01-02 00:00:00Z Sunday
  dt = OffsetDateTime::forComponents(0, 1, 2, 0, 0, 0);
  assertEqual((uint32_t) 1, dt.toEpochDays());
  assertEqual((uint32_t) 86400, dt.toEpochSeconds());
  assertEqual(LocalDate::kSunday, dt.dayOfWeek());

  // 2018-01-01 00:00:00Z Monday
  dt = OffsetDateTime::forComponents(18, 1, 1, 0, 0, 0);
  assertEqual((uint32_t) 6575, dt.toEpochDays());
  assertEqual(6575 * (uint32_t) 86400, dt.toEpochSeconds());
  assertEqual(LocalDate::kMonday, dt.dayOfWeek());

  // 2018-01-01 00:00:00+00:15 Monday
  dt = OffsetDateTime::forComponents(18, 1, 1, 0, 0, 0,
      ZoneOffset::forOffsetCode(1));
  assertEqual((uint32_t) 6574, dt.toEpochDays());
  assertEqual(6575 * (uint32_t) 86400 - 15*60, dt.toEpochSeconds());
  assertEqual(LocalDate::kMonday, dt.dayOfWeek());

  // 2049-12-31 23:59:59Z Friday
  dt = OffsetDateTime::forComponents(49, 12, 31, 23, 59, 59);
  assertEqual((uint32_t) 18262, dt.toEpochDays());
  assertEqual(18263 * (uint32_t) 86400 - 1, dt.toEpochSeconds());
  assertEqual(LocalDate::kFriday, dt.dayOfWeek());

  // 2049-12-31 23:59:59-00:15 Friday
  dt = OffsetDateTime::forComponents(49, 12, 31, 23, 59, 59,
      ZoneOffset::forOffsetCode(-1));
  assertEqual((uint32_t) 18263, dt.toEpochDays());
  assertEqual(18263 * (uint32_t) 86400 + 15*60 - 1, dt.toEpochSeconds());
  assertEqual(LocalDate::kFriday, dt.dayOfWeek());

  // 2050-01-01 00:00:00Z Saturday
  dt = OffsetDateTime::forComponents(50, 1, 1, 0, 0, 0);
  assertEqual((uint32_t) 18263, dt.toEpochDays());
  assertEqual(18263 * (uint32_t) 86400, dt.toEpochSeconds());
  assertEqual(LocalDate::kSaturday, dt.dayOfWeek());

  // 2099-12-31 23:59:59Z Thursday
  dt = OffsetDateTime::forComponents(99, 12, 31, 23, 59, 59);
  assertEqual((uint32_t) 36524, dt.toEpochDays());
  assertEqual(36525 * (uint32_t) 86400 - 1, dt.toEpochSeconds());
  assertEqual(LocalDate::kThursday, dt.dayOfWeek());
}

test(OffsetDateTimeTest, toUnixSeconds) {
  // 2000-01-01 00:00:00Z
  OffsetDateTime dt = OffsetDateTime::forComponents(0, 1, 1, 0, 0, 0);
  assertEqual((uint32_t) 946684800, dt.toUnixSeconds());

  // 2018-01-01 00:00:00Z
  dt = OffsetDateTime::forComponents(18, 1, 1, 0, 0, 0);
  assertEqual((uint32_t) 1514764800, dt.toUnixSeconds());

  // 2018-08-30T06:45:01-07:00
  dt = OffsetDateTime::forComponents(18, 8, 30, 6, 45, 1,
      ZoneOffset::forHour(-7));
  assertEqual((uint32_t) 1535636701, dt.toUnixSeconds());

  // 2038-01-01 00:00:00Z
  dt = OffsetDateTime::forComponents(38, 1, 1, 0, 0, 0);
  assertEqual((uint32_t) 2145916800, dt.toUnixSeconds());

  // 2099-12-31 23:59:59-16:00
  dt = OffsetDateTime::forComponents(99, 12, 31, 23, 59, 59,
      ZoneOffset::forHour(-16));
  assertEqual((uint32_t) 4102502399, dt.toUnixSeconds());
}

test(OffsetDateTimeTest, forEpochSeconds) {
  // 2049-12-31 23:59:59Z Friday
  OffsetDateTime dt = OffsetDateTime::forEpochSeconds(
      18263 * (int32_t) 86400 - 1);

  assertEqual((uint16_t) 2049, dt.yearFull());
  assertEqual(49, dt.year());
  assertEqual(12, dt.month());
  assertEqual(31, dt.day());
  assertEqual(23, dt.hour());
  assertEqual(59, dt.minute());
  assertEqual(59, dt.second());
  assertEqual(LocalDate::kFriday, dt.dayOfWeek());

  // 2049-12-31 15:59:59-08:00 Friday
  ZoneOffset offset = ZoneOffset::forOffsetCode(-32); // UTC-08:00
  dt = OffsetDateTime::forEpochSeconds(18263 * (int32_t) 86400 - 1, offset);
  assertEqual((uint16_t) 2049, dt.yearFull());
  assertEqual(49, dt.year());
  assertEqual(12, dt.month());
  assertEqual(31, dt.day());
  assertEqual(15, dt.hour());
  assertEqual(59, dt.minute());
  assertEqual(59, dt.second());
  assertEqual(LocalDate::kFriday, dt.dayOfWeek());
}

test(OffsetDateTimeTest, convertToZoneOffset) {
  OffsetDateTime a = OffsetDateTime::forComponents(18, 1, 1, 12, 0, 0);
  OffsetDateTime b = a.convertToZoneOffset(ZoneOffset::forHour(-7));

  assertEqual((uint16_t) 2018, b.yearFull());
  assertEqual(18, b.year());
  assertEqual(1, b.month());
  assertEqual(1, b.day());
  assertEqual(5, b.hour());
  assertEqual(0, b.minute());
  assertEqual(0, b.second());
  assertEqual(-28, b.zoneOffset().toOffsetCode());
}

test(OffsetDateTimeTest, compareTo) {
  OffsetDateTime a = OffsetDateTime::forComponents(18, 1, 1, 12, 0, 0);
  OffsetDateTime b = OffsetDateTime::forComponents(18, 1, 1, 12, 0, 0);
  assertEqual(a.compareTo(b), 0);
  assertTrue(a == b);
  assertFalse(a != b);

  a = OffsetDateTime::forComponents(18, 1, 1, 12, 0, 0);
  b = OffsetDateTime::forComponents(18, 1, 1, 12, 0, 1);
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);

  a = OffsetDateTime::forComponents(18, 1, 1, 12, 0, 0);
  b = OffsetDateTime::forComponents(18, 1, 1, 12, 1, 0);
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);

  a = OffsetDateTime::forComponents(18, 1, 1, 11, 0, 0);
  b = OffsetDateTime::forComponents(18, 1, 1, 12, 0, 0,
      ZoneOffset::forOffsetCode(1));
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);

  a = OffsetDateTime::forComponents(18, 1, 1, 12, 0, 0);
  b = OffsetDateTime::forComponents(18, 1, 2, 12, 0, 0);
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);

  a = OffsetDateTime::forComponents(18, 1, 1, 12, 0, 0);
  b = OffsetDateTime::forComponents(18, 2, 1, 12, 0, 0);
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);

  a = OffsetDateTime::forComponents(18, 1, 1, 12, 0, 0);
  b = OffsetDateTime::forComponents(19, 1, 1, 12, 0, 0);
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);

  // 2018-1-1 12:00:00+01:00
  a = OffsetDateTime::forComponents(18, 1, 1, 12, 0, 0, ZoneOffset::forHour(1));
  // 2018-1-1 12:00:00-08:00
  b = OffsetDateTime::forComponents(18, 1, 1, 12, 0, 0,
      ZoneOffset::forHour(-8));
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);
}

test(OffsetDateTimeTest, dayOfWeek) {
  // 2018-01-01 00:00:00Z Monday
  OffsetDateTime dt = OffsetDateTime::forComponents(18, 1, 1, 0, 0, 0);
  assertEqual(LocalDate::kMonday, dt.dayOfWeek());

  dt.hour(23); // 2018-01-01 23:00:00Z, no change to dayOfWeek
  assertEqual(LocalDate::kMonday, dt.dayOfWeek());

  dt.minute(40); // 2018-01-01 23:40:00Z, no change to dayOfWeek
  assertEqual(LocalDate::kMonday, dt.dayOfWeek());

  dt.second(3); // 2018-01-01 23:40:03Z, no change to dayOfWeek
  assertEqual(LocalDate::kMonday, dt.dayOfWeek());

  // 2018-01-01 23:40:03+00:45, no change to dayOfWeek
  dt.zoneOffset(ZoneOffset::forOffsetCode(3));
  assertEqual(LocalDate::kMonday, dt.dayOfWeek());

  dt.day(2); // 2018-01-02 23:40:03+00:45, changes dayOfWeek
  assertEqual(LocalDate::kTuesday, dt.dayOfWeek());

  dt.month(2); // 2018-02-02 23:40:03+00:45, changes dayOfWeek
  assertEqual(LocalDate::kFriday, dt.dayOfWeek());

  dt.year(19); // 2019-02-02 23:40:03+00:45, changes dayOfWeek
  assertEqual(LocalDate::kSaturday, dt.dayOfWeek());

  dt.yearFull(2020); // 2020-02-02 23:40:03+00:45, changes dayOfWeek
  assertEqual(LocalDate::kSunday, dt.dayOfWeek());
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

  // parser cares about UTC+/- offset
  dt = OffsetDateTime::forDateString(F("2018-08-31 13:48:01&07:00"));
  assertTrue(dt.isError());
}

test(OffsetDateTimeTest, forDateString) {
  // exact ISO8601 format
  OffsetDateTime dt = OffsetDateTime::forDateString(
      F("2018-08-31T13:48:01-07:00"));
  assertFalse(dt.isError());
  assertEqual((uint16_t) 2018, dt.yearFull());
  assertEqual(18, dt.year());
  assertEqual(8, dt.month());
  assertEqual(31, dt.day());
  assertEqual(13, dt.hour());
  assertEqual(48, dt.minute());
  assertEqual(1, dt.second());
  assertEqual(-28, dt.zoneOffset().toOffsetCode());
  assertEqual(LocalDate::kFriday, dt.dayOfWeek());

  // parser does not care about most separators, this may change in the future
  dt = OffsetDateTime::forDateString(F("2018/08/31 13#48#01+07#00"));
  assertFalse(dt.isError());
  assertEqual(18, dt.year());
  assertEqual(8, dt.month());
  assertEqual(31, dt.day());
  assertEqual(13, dt.hour());
  assertEqual(48, dt.minute());
  assertEqual(1, dt.second());
  assertEqual(28, dt.zoneOffset().toOffsetCode());
  assertEqual(LocalDate::kFriday, dt.dayOfWeek());
}

// --------------------------------------------------------------------------
// TimePeriod
// --------------------------------------------------------------------------

test(TimePeriodTest, fromComponents) {
  TimePeriod t(0, 16, 40, 1);
  assertEqual(t.toSeconds(), (int32_t) 1000);

  TimePeriod u(0, 16, 40, -1);
  assertEqual(u.toSeconds(), (int32_t) -1000);
}

test(TimePeriodTest, fromSeconds) {
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

test(TimePeriodTest, compareAndEquals) {
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

test(TimePeriodTest, incrementHour) {
  TimePeriod a(3, 2, 1, 1);
  a.incrementHour();
  TimePeriod expected(4, 2, 1, 1);
  assertTrue(expected == a);

  a = TimePeriod(23, 2, 1, -1);
  a.incrementHour();
  expected = TimePeriod(0, 2, 1, -1);
  assertTrue(expected == a);
}

test(TimePeriodTest, incrementMinute) {
  TimePeriod a(3, 2, 1, 1);
  a.incrementMinute();
  TimePeriod expected(3, 3, 1, 1);
  assertTrue(expected == a);

  a = TimePeriod(3, 59, 1, 1);
  a.incrementMinute();
  expected = TimePeriod(3, 0, 1, 1);
  assertTrue(expected == a);
}

test(TimePeriodTest, negate) {
  TimePeriod a(3, 2, 1, 1);
  assertEqual((int8_t) 1, a.sign());
  a.negate();
  assertEqual((int8_t) -1, a.sign());
}

// --------------------------------------------------------------------------
// ZoneOffset
// --------------------------------------------------------------------------

test(ZoneOffsetTest, forOffsetCode) {
  ZoneOffset offset = ZoneOffset::forOffsetCode(-1);
  assertEqual((int16_t) -15, offset.toMinutes());
  assertEqual((int32_t) -900, offset.toSeconds());

  offset = ZoneOffset::forOffsetCode(1);
  assertEqual((int16_t) 15, offset.toMinutes());
  assertEqual((int32_t) 900, offset.toSeconds());
}

test(ZoneOffsetTest, forHour) {
  assertEqual(ZoneOffset::forHour(-8).toOffsetCode(), -32);
  assertEqual(ZoneOffset::forHour(1).toOffsetCode(), 4);
}

test(ZoneOffsetTest, forHourMinute) {
  assertEqual(ZoneOffset::forHourMinute(-1, 8, 0).toOffsetCode(), -32);
  assertEqual(ZoneOffset::forHourMinute(-1, 8, 15).toOffsetCode(), -33);
  assertEqual(ZoneOffset::forHourMinute(1, 1, 0).toOffsetCode(), 4);
  assertEqual(ZoneOffset::forHourMinute(1, 1, 15).toOffsetCode(), 5);
}

test(ZoneOffsetTest, forOffsetString) {
  assertTrue(ZoneOffset::forOffsetString("").isError());
  assertEqual(ZoneOffset::forOffsetString("-07:00").toOffsetCode(), -28);
  assertEqual(ZoneOffset::forOffsetString("-07:45").toOffsetCode(), -31);
  assertEqual(ZoneOffset::forOffsetString("+01:00").toOffsetCode(), 4);
  assertEqual(ZoneOffset::forOffsetString("+01:15").toOffsetCode(), 5);
  assertEqual(ZoneOffset::forOffsetString("+01:16").toOffsetCode(), 5);
}

test(ZoneOffsetTest, error) {
  ZoneOffset offset;
  assertFalse(offset.isError());

  offset.setError();
  assertTrue(offset.isError());
}

test(ZoneOffsetTest, incrementHour) {
  ZoneOffset offset = ZoneOffset::forOffsetCode(-1);
  offset.incrementHour();
  assertEqual((int8_t) 3, offset.toOffsetCode());

  offset = ZoneOffset::forOffsetCode(63);
  offset.incrementHour();
  assertEqual((int8_t) -61, offset.toOffsetCode());

  offset = ZoneOffset::forOffsetCode(60);
  offset.incrementHour();
  assertEqual((int8_t) -64, offset.toOffsetCode());
}

test(ZoneOffsetTest, increment15Minutes) {
  ZoneOffset offset = ZoneOffset::forOffsetCode(3);

  offset.increment15Minutes();
  assertEqual((int8_t) 0, offset.toOffsetCode());

  offset.increment15Minutes();
  assertEqual((int8_t) 1, offset.toOffsetCode());

  offset.increment15Minutes();
  assertEqual((int8_t) 2, offset.toOffsetCode());

  offset.increment15Minutes();
  assertEqual((int8_t) 3, offset.toOffsetCode());

  offset = ZoneOffset::forOffsetCode(-4);
  offset.increment15Minutes();
  assertEqual((int8_t) -5, offset.toOffsetCode());

  offset.increment15Minutes();
  assertEqual((int8_t) -6, offset.toOffsetCode());

  offset.increment15Minutes();
  assertEqual((int8_t) -7, offset.toOffsetCode());

  offset.increment15Minutes();
  assertEqual((int8_t) -4, offset.toOffsetCode());
}

test(ZoneOffsetTest, convertOffsetCode) {
  ZoneOffset zoneOffset = ZoneOffset::forOffsetCode(-29);
  int8_t sign;
  uint8_t hour;
  uint8_t minute;
  zoneOffset.toHourMinute(sign, hour, minute);
  assertEqual(-1, sign);
  assertEqual(7, hour);
  assertEqual(15, minute);
}

// --------------------------------------------------------------------------
// TimeZone
// --------------------------------------------------------------------------

test(TimeZoneTest, dstOffset) {
  TimeZone pdt = TimeZone::forZoneOffset(ZoneOffset::forHour(-8)).isDst(true);
  assertTrue(pdt.isDst());
  assertEqual(pdt.zoneOffset().toOffsetCode(), -32);
  assertEqual(pdt.effectiveZoneOffset(0).toOffsetCode(), -28);
}

test(TimeZoneTest, setError) {
  TimeZone error = TimeZone::forZoneOffset(ZoneOffset::forHour(-8)).setError();
  assertTrue(error.isError());
}

test(timeZoneZoneInfo) {
  uint32_t now = 593481600; // 2018-10-21
  TimeZone losAngeles = TimeZone::forZone(&ZoneInfo::kZoneInfoLosAngeles);
  ZoneOffset offset = losAngeles.effectiveZoneOffset(now);
  assertEqual(offset.toOffsetCode(), -28);
}

// --------------------------------------------------------------------------
// DateStrings
// --------------------------------------------------------------------------

test(DateStringsTest, monthString) {
  DateStrings ds;

  assertEqual(F("Error"), ds.monthLongString(0));
  assertEqual(F("January"), ds.monthLongString(1));
  assertEqual(F("February"), ds.monthLongString(2));
  assertEqual(F("March"), ds.monthLongString(3));
  assertEqual(F("April"), ds.monthLongString(4));
  assertEqual(F("May"), ds.monthLongString(5));
  assertEqual(F("June"), ds.monthLongString(6));
  assertEqual(F("July"), ds.monthLongString(7));
  assertEqual(F("August"), ds.monthLongString(8));
  assertEqual(F("September"), ds.monthLongString(9));
  assertEqual(F("October"), ds.monthLongString(10));
  assertEqual(F("November"), ds.monthLongString(11));
  assertEqual(F("December"), ds.monthLongString(12));
  assertEqual(F("Error"), ds.monthLongString(13));

  assertEqual(F("Err"), ds.monthShortString(0));
  assertEqual(F("Jan"), ds.monthShortString(1));
  assertEqual(F("Feb"), ds.monthShortString(2));
  assertEqual(F("Mar"), ds.monthShortString(3));
  assertEqual(F("Apr"), ds.monthShortString(4));
  assertEqual(F("May"), ds.monthShortString(5));
  assertEqual(F("Jun"), ds.monthShortString(6));
  assertEqual(F("Jul"), ds.monthShortString(7));
  assertEqual(F("Aug"), ds.monthShortString(8));
  assertEqual(F("Sep"), ds.monthShortString(9));
  assertEqual(F("Oct"), ds.monthShortString(10));
  assertEqual(F("Nov"), ds.monthShortString(11));
  assertEqual(F("Dec"), ds.monthShortString(12));
  assertEqual(F("Err"), ds.monthShortString(13));
}

test(DateStringsTest, monthStringsFitInBuffer) {
  DateStrings ds;
  uint8_t maxLength = 0;
  for (uint8_t month = 0; month <= 12; month++) {
    const char* monthString = ds.monthLongString(month);
    uint8_t length = strlen(monthString);
    if (length > maxLength) { maxLength = length; }
  }
  assertLessOrEqual(maxLength, DateStrings::kBufferSize - 1);
}

test(DateStringsTest, weekDayStrings) {
  DateStrings ds;

  assertEqual(F("Error"), ds.weekDayLongString(0));
  assertEqual(F("Monday"), ds.weekDayLongString(1));
  assertEqual(F("Tuesday"), ds.weekDayLongString(2));
  assertEqual(F("Wednesday"), ds.weekDayLongString(3));
  assertEqual(F("Thursday"), ds.weekDayLongString(4));
  assertEqual(F("Friday"), ds.weekDayLongString(5));
  assertEqual(F("Saturday"), ds.weekDayLongString(6));
  assertEqual(F("Sunday"), ds.weekDayLongString(7));
  assertEqual(F("Error"), ds.weekDayLongString(8));

  assertEqual(F("Err"), ds.weekDayShortString(0));
  assertEqual(F("Mon"), ds.weekDayShortString(1));
  assertEqual(F("Tue"), ds.weekDayShortString(2));
  assertEqual(F("Wed"), ds.weekDayShortString(3));
  assertEqual(F("Thu"), ds.weekDayShortString(4));
  assertEqual(F("Fri"), ds.weekDayShortString(5));
  assertEqual(F("Sat"), ds.weekDayShortString(6));
  assertEqual(F("Sun"), ds.weekDayShortString(7));
  assertEqual(F("Err"), ds.weekDayShortString(8));
}

test(DateStringsTest, weekDayStringsFitInBuffer) {
  DateStrings ds;
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
