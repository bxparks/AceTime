#line 2 "AceTimeTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time;

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
// UtcOffset
// --------------------------------------------------------------------------

test(UtcOffsetTest, code) {
  assertEqual(UtcOffset::forHour(-8).code(), -8*4);
}

test(UtcOffsetTest, isDst) {
  assertFalse(UtcOffset::forHour(0).isDst());
  assertFalse(UtcOffset().isDst());
}

test(UtcOffsetTest, forMinutes) {
  UtcOffset offset = UtcOffset::forMinutes(-15);
  assertEqual((int16_t) -15, offset.toMinutes());
  assertEqual((int32_t) -900, offset.toSeconds());

  offset = UtcOffset::forMinutes(15);
  assertEqual((int16_t) 15, offset.toMinutes());
  assertEqual((int32_t) 900, offset.toSeconds());

  offset = UtcOffset::forMinutes(-16);
  assertEqual((int16_t) -15, offset.toMinutes());
  assertEqual((int32_t) -900, offset.toSeconds());

  offset = UtcOffset::forMinutes(16);
  assertEqual((int16_t) 15, offset.toMinutes());
  assertEqual((int32_t) 900, offset.toSeconds());
}

test(UtcOffsetTest, forHour) {
  assertEqual(UtcOffset::forHour(-8).toMinutes(), -8*60);
  assertEqual(UtcOffset::forHour(1).toMinutes(), 1*60);
}

test(UtcOffsetTest, forHourMinute) {
  assertEqual(UtcOffset::forHourMinute(-1, 8, 0).toMinutes(), -(8*60));
  assertEqual(UtcOffset::forHourMinute(-1, 8, 15).toMinutes(), -(8*60+15));
  assertEqual(UtcOffset::forHourMinute(1, 1, 0).toMinutes(), 60);
  assertEqual(UtcOffset::forHourMinute(1, 1, 15).toMinutes(), 75);
}

test(UtcOffsetTest, forOffsetString) {
  assertTrue(UtcOffset::forOffsetString("").isError());
  assertEqual(UtcOffset::forOffsetString("-07:00").toMinutes(), -7*60);
  assertEqual(UtcOffset::forOffsetString("-07:45").toMinutes(), -(7*60+45));
  assertEqual(UtcOffset::forOffsetString("+01:00").toMinutes(), 60);
  assertEqual(UtcOffset::forOffsetString("+01:15").toMinutes(), 75);
  assertEqual(UtcOffset::forOffsetString("+01:16").toMinutes(), 75);
}

test(UtcOffsetTest, error) {
  UtcOffset offset;
  assertFalse(offset.isError());

  offset = UtcOffset::forError();
  assertTrue(offset.isError());
}

// --------------------------------------------------------------------------
// utc_offset_mutation
// --------------------------------------------------------------------------

test(UtcOffsetMutationTest, incrementHour) {
  int8_t sign;
  uint8_t hour;
  uint8_t minute;

  UtcOffset offset = UtcOffset::forMinutes(0);
  utc_offset_mutation::incrementHour(offset);
  assertEqual((int16_t) 60, offset.toMinutes());

  offset = UtcOffset::forHourMinute(1, 1, 45);
  utc_offset_mutation::incrementHour(offset);
  offset.toHourMinute(sign, hour, minute);
  assertEqual(1, sign);
  assertEqual(2, hour);
  assertEqual(45, minute);

  // Wrap around at 16h to -16h, but keep the minutes the same.
  offset = UtcOffset::forHourMinute(1, 15, 45);
  utc_offset_mutation::incrementHour(offset);
  offset.toHourMinute(sign, hour, minute);
  assertEqual(-1, sign);
  assertEqual(15, hour);
  assertEqual(45, minute);

  utc_offset_mutation::incrementHour(offset);
  offset.toHourMinute(sign, hour, minute);
  assertEqual(-1, sign);
  assertEqual(14, hour);
  assertEqual(45, minute);
}

test(UtcOffsetMutationTest, increment15Minutes) {
  int8_t sign;
  uint8_t hour;
  uint8_t minute;

  UtcOffset offset = UtcOffset::forHourMinute(1, 0, 0);

  utc_offset_mutation::increment15Minutes(offset);
  offset.toHourMinute(sign, hour, minute);
  assertEqual(1, sign);
  assertEqual(0, hour);
  assertEqual(15, minute);

  utc_offset_mutation::increment15Minutes(offset);
  offset.toHourMinute(sign, hour, minute);
  assertEqual(1, sign);
  assertEqual(0, hour);
  assertEqual(30, minute);

  utc_offset_mutation::increment15Minutes(offset);
  offset.toHourMinute(sign, hour, minute);
  assertEqual(1, sign);
  assertEqual(0, hour);
  assertEqual(45, minute);

  // Wrap the minute.
  utc_offset_mutation::increment15Minutes(offset);
  offset.toHourMinute(sign, hour, minute);
  assertEqual(1, sign);
  assertEqual(0, hour);
  assertEqual(0, minute);

  offset = UtcOffset::forHourMinute(-1, 1, 0);
  utc_offset_mutation::increment15Minutes(offset);
  offset.toHourMinute(sign, hour, minute);
  assertEqual(-1, sign);
  assertEqual(1, hour);
  assertEqual(15, minute);

  utc_offset_mutation::increment15Minutes(offset);
  offset.toHourMinute(sign, hour, minute);
  assertEqual(-1, sign);
  assertEqual(1, hour);
  assertEqual(30, minute);

  utc_offset_mutation::increment15Minutes(offset);
  offset.toHourMinute(sign, hour, minute);
  assertEqual(-1, sign);
  assertEqual(1, hour);
  assertEqual(45, minute);

  utc_offset_mutation::increment15Minutes(offset);
  offset.toHourMinute(sign, hour, minute);
  assertEqual(-1, sign);
  assertEqual(1, hour);
  assertEqual(0, minute);
}

// --------------------------------------------------------------------------
// DateStrings
// --------------------------------------------------------------------------

test(DateStringsTest, monthString) {
  common::DateStrings ds;

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
  common::DateStrings ds;
  uint8_t maxLength = 0;
  for (uint8_t month = 0; month <= 12; month++) {
    const char* monthString = ds.monthLongString(month);
    uint8_t length = strlen(monthString);
    if (length > maxLength) { maxLength = length; }
  }
  assertLessOrEqual(maxLength, common::DateStrings::kBufferSize - 1);
}

test(DateStringsTest, weekDayStrings) {
  common::DateStrings ds;

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
  common::DateStrings ds;
  uint8_t maxLength = 0;
  for (uint8_t weekDay = 0; weekDay <= 7; weekDay++) {
    const char* weekDayString = ds.weekDayLongString(weekDay);
    uint8_t length = strlen(weekDayString);
    if (length > maxLength) { maxLength = length; }
  }
  assertLessOrEqual(maxLength, common::DateStrings::kBufferSize - 1);
}

// --------------------------------------------------------------------------

void setup() {
#if !defined(__linux__) && !defined(__APPLE__)
  delay(1000); // wait for stability on some boards to prevent garbage Serial
#endif
  Serial.begin(115200); // ESP8266 default of 74880 not supported on Linux
  while(!Serial); // for the Arduino Leonardo/Micro only
}

void loop() {
  TestRunner::run();
}
