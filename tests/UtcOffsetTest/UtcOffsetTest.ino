#line 2 "UtcOffsetTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time;

// --------------------------------------------------------------------------
// UtcOffset
// --------------------------------------------------------------------------

test(UtcOffsetTest, code) {
  assertEqual(UtcOffset::forHour(-8).toOffsetCode(), -8*4);
}

test(UtcOffsetTest, isZero) {
  assertTrue(UtcOffset::forHour(0).isZero());
  assertTrue(UtcOffset().isZero());
  assertFalse(UtcOffset::forHour(1).isZero());
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
  assertEqual(UtcOffset::forHourMinute(-8, 0).toMinutes(), -(8*60));
  assertEqual(UtcOffset::forHourMinute(-8, 15).toMinutes(), -(8*60+15));
  assertEqual(UtcOffset::forHourMinute(1, 0).toMinutes(), 60);
  assertEqual(UtcOffset::forHourMinute(1, 15).toMinutes(), 75);
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

  offset = UtcOffset::forHourMinute(1, 45);
  utc_offset_mutation::incrementHour(offset);
  offset.toHourMinute(sign, hour, minute);
  assertEqual(1, sign);
  assertEqual(2, hour);
  assertEqual(45, minute);

  // Wrap around at 16h to -16h, but keep the minutes the same.
  offset = UtcOffset::forHourMinute(15, 45);
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

  UtcOffset offset;

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

  offset = UtcOffset::forHourMinute(-1, 0);
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
