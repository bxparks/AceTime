#line 2 "DateTimeTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time;

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

test(OffsetDateTimeTest, increment) {
  OffsetDateTime dt = OffsetDateTime::forComponents(1, 2, 3, 4, 5, 6);
  assertEqual(1, dt.year());
  assertEqual(2, dt.month());
  assertEqual(3, dt.day());
  assertEqual(4, dt.hour());
  assertEqual(5, dt.minute());
  assertEqual(6, dt.second());
  assertEqual(0, dt.zoneOffset().toOffsetCode());

  dt.incrementYear();
  assertEqual(2, dt.year());
  assertEqual(2, dt.month());
  assertEqual(3, dt.day());
  assertEqual(4, dt.hour());
  assertEqual(5, dt.minute());
  assertEqual(6, dt.second());
  assertEqual(0, dt.zoneOffset().toOffsetCode());

  dt.incrementMonth();
  assertEqual(2, dt.year());
  assertEqual(3, dt.month());
  assertEqual(3, dt.day());
  assertEqual(4, dt.hour());
  assertEqual(5, dt.minute());
  assertEqual(6, dt.second());
  assertEqual(0, dt.zoneOffset().toOffsetCode());

  dt.incrementDay();
  assertEqual(2, dt.year());
  assertEqual(3, dt.month());
  assertEqual(4, dt.day());
  assertEqual(4, dt.hour());
  assertEqual(5, dt.minute());
  assertEqual(6, dt.second());
  assertEqual(0, dt.zoneOffset().toOffsetCode());

  dt.incrementHour();
  assertEqual(2, dt.year());
  assertEqual(3, dt.month());
  assertEqual(4, dt.day());
  assertEqual(5, dt.hour());
  assertEqual(5, dt.minute());
  assertEqual(6, dt.second());
  assertEqual(0, dt.zoneOffset().toOffsetCode());

  dt.incrementMinute();
  assertEqual(2, dt.year());
  assertEqual(3, dt.month());
  assertEqual(4, dt.day());
  assertEqual(5, dt.hour());
  assertEqual(6, dt.minute());
  assertEqual(6, dt.second());
  assertEqual(0, dt.zoneOffset().toOffsetCode());
}

// --------------------------------------------------------------------------
// DateTime
// --------------------------------------------------------------------------

test(DateTimeTest, forComponents_beforeDst) {
  TimeZone tz = TimeZone::forZone(&zonedb::kLos_Angeles);
  DateTime dt = DateTime::forComponents(18, 3, 11, 1, 59, 59, tz);

  ZoneOffset pst = ZoneOffset::forHour(-8);
  OffsetDateTime otz = OffsetDateTime::forComponents(18, 3, 11, 1, 59, 59, pst);

  assertEqual(otz.toEpochSeconds(), dt.toEpochSeconds());
}

test(DateTimeTest, forComponents_inDstGap) {
  TimeZone tz = TimeZone::forZone(&zonedb::kLos_Angeles);
  DateTime dt = DateTime::forComponents(18, 3, 11, 2, 0, 1, tz);

  ZoneOffset pdt = ZoneOffset::forHour(-7);
  OffsetDateTime odt = OffsetDateTime::forComponents(18, 3, 11, 2, 0, 1, pdt);

  assertEqual(odt.toEpochSeconds(), dt.toEpochSeconds());
}

test(DateTimeTest, forComponents_inDst) {
  TimeZone tz = TimeZone::forZone(&zonedb::kLos_Angeles);
  DateTime dt = DateTime::forComponents(18, 3, 11, 3, 0, 1, tz);

  ZoneOffset pdt = ZoneOffset::forHour(-7);
  OffsetDateTime odt = OffsetDateTime::forComponents(18, 3, 11, 3, 0, 1, pdt);

  assertEqual(odt.toEpochSeconds(), dt.toEpochSeconds());
}

test(DateTimeTest, forComponents_beforeStd) {
  TimeZone tz = TimeZone::forZone(&zonedb::kLos_Angeles);
  DateTime dt = DateTime::forComponents(18, 11, 4, 0, 59, 59, tz);

  ZoneOffset pdt = ZoneOffset::forHour(-7);
  OffsetDateTime odt = OffsetDateTime::forComponents(18, 11, 4, 0, 59, 59, pdt);

  assertEqual(odt.toEpochSeconds(), dt.toEpochSeconds());
}

test(DateTimeTest, forComponents_inOverlap) {
  TimeZone tz = TimeZone::forZone(&zonedb::kLos_Angeles);
  DateTime dt = DateTime::forComponents(18, 11, 4, 1, 0, 1, tz); // ambiguous

  ZoneOffset pdt = ZoneOffset::forHour(-8);
  OffsetDateTime odt = OffsetDateTime::forComponents(18, 11, 4, 1, 0, 1, pdt);

  assertEqual(odt.toEpochSeconds(), dt.toEpochSeconds());
}


test(DateTimeTest, forComponents_afterOverlap) {
  TimeZone tz = TimeZone::forZone(&zonedb::kLos_Angeles);
  DateTime dt = DateTime::forComponents(18, 11, 4, 2, 0, 1, tz); // ambiguous

  ZoneOffset pdt = ZoneOffset::forHour(-8);
  OffsetDateTime odt = OffsetDateTime::forComponents(18, 11, 4, 2, 0, 1, pdt);

  assertEqual(odt.toEpochSeconds(), dt.toEpochSeconds());
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
