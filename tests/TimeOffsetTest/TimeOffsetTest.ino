#line 2 "TimeOffsetTest.ino"

#include <AUnit.h>
#include <AceTime.h>
#include <AceCommon.h> // PrintStr<N>

using namespace ace_time;
using ace_common::PrintStr;

//---------------------------------------------------------------------------
// TimeOffset
//---------------------------------------------------------------------------

test(TimeOffsetTest, operatorEqualEqual) {
  TimeOffset a = TimeOffset::forMinutes(10);
  TimeOffset aa = TimeOffset::forMinutes(10);
  TimeOffset b = TimeOffset::forMinutes(11);
  assertTrue(a == aa);
  assertTrue(a != b);
}

test(TimeOffsetTest, isZero) {
  assertTrue(TimeOffset::forHours(0).isZero());
  assertTrue(TimeOffset().isZero());
  assertFalse(TimeOffset::forHours(1).isZero());
}

test(TimeOffsetTest, forMinutes) {
  TimeOffset offset = TimeOffset::forMinutes(-15);
  assertEqual((int16_t) -15, offset.toMinutes());
  assertEqual((int32_t) -900, offset.toSeconds());

  offset = TimeOffset::forMinutes(15);
  assertEqual((int16_t) 15, offset.toMinutes());
  assertEqual((int32_t) 900, offset.toSeconds());

  offset = TimeOffset::forMinutes(-16);
  assertEqual((int16_t) -16, offset.toMinutes());
  assertEqual((int32_t) -960, offset.toSeconds());

  offset = TimeOffset::forMinutes(16);
  assertEqual((int16_t) 16, offset.toMinutes());
  assertEqual((int32_t) 960, offset.toSeconds());
}

test(TimeOffsetTest, forSeconds) {
  TimeOffset offset = TimeOffset::forSeconds(-901);
  assertEqual((int16_t) -15, offset.toMinutes());
  assertEqual((int32_t) -901, offset.toSeconds());

  offset = TimeOffset::forSeconds(902);
  assertEqual((int16_t) 15, offset.toMinutes());
  assertEqual((int32_t) 902, offset.toSeconds());

  offset = TimeOffset::forSeconds(-963);
  assertEqual((int16_t) -16, offset.toMinutes());
  assertEqual((int32_t) -963, offset.toSeconds());

  offset = TimeOffset::forSeconds(3601);
  assertEqual((int16_t) 60, offset.toMinutes());
  assertEqual((int32_t) 3601, offset.toSeconds());
}

test(TimeOffsetTest, forHour) {
  assertEqual(TimeOffset::forHours(-8).toMinutes(), -8*60);
  assertEqual(TimeOffset::forHours(1).toMinutes(), 1*60);
}

test(TimeOffsetTest, forHourMinute) {
  assertEqual(TimeOffset::forHourMinute(-8, 0).toMinutes(), -(8*60));
  assertEqual(TimeOffset::forHourMinute(-8, -15).toMinutes(), -(8*60+15));
  assertEqual(TimeOffset::forHourMinute(1, 0).toMinutes(), 60);
  assertEqual(TimeOffset::forHourMinute(1, 15).toMinutes(), 75);
  assertEqual(TimeOffset::forHourMinute(0, -15).toMinutes(), -15);
}

test(TimeOffsetTest, forHourMinuteSecond) {
  assertEqual(
      TimeOffset::forHourMinuteSecond(8, 0, 1).toSeconds(), 8*3600+1);
  assertEqual(
      TimeOffset::forHourMinuteSecond(-8, -15, -1).toSeconds(),
      -(60*(8*60+15)+1));
  assertEqual(
      TimeOffset::forHourMinuteSecond(1, 2, 3).toSeconds(), 60*(60*1+2)+3);
  assertEqual(
      TimeOffset::forHourMinuteSecond(1, 15, 1).toSeconds(), 60*(60*1+15)+1);
  assertEqual(
      TimeOffset::forHourMinuteSecond(0, -15, -1).toSeconds(), -15*60-1);
}

test(TimeOffsetTest, forOffsetString) {
  assertTrue(TimeOffset::forOffsetString("").isError());

  assertEqual(TimeOffset::forOffsetString("-07:00").toMinutes(), -7*60);
  assertEqual(TimeOffset::forOffsetString("-07:45").toMinutes(), -(7*60+45));
  assertEqual(TimeOffset::forOffsetString("+01:00").toMinutes(), 60);
  assertEqual(TimeOffset::forOffsetString("+01:15").toMinutes(), 75);
  assertEqual(TimeOffset::forOffsetString("+01:16").toMinutes(), 76);

  assertEqual(
      TimeOffset::forOffsetString("-07:00:01").toSeconds(), -7*3600-1);
  assertEqual(
      TimeOffset::forOffsetString("-07:45:02").toSeconds(), -((7*60+45)*60+2));
  assertEqual(
      TimeOffset::forOffsetString("+01:00:03").toSeconds(), 3600+3);
  assertEqual(
      TimeOffset::forOffsetString("+01:15:04").toSeconds(), 3600+15*60+4);
  assertEqual(
      TimeOffset::forOffsetString("+01:16:05").toSeconds(), 3600+16*60+5);
}

test(TimeOffsetTest, printTo) {
  PrintStr<32> str;
  TimeOffset::forHourMinuteSecond(1, 2, 0).printTo(str);
  assertEqual(str.cstr(), "+01:02");

  str.flush();
  TimeOffset::forHourMinuteSecond(1, 2, 3).printTo(str);
  assertEqual(str.cstr(), "+01:02:03");

  str.flush();
  TimeOffset::forHourMinuteSecond(-1, -2, -3).printTo(str);
  assertEqual(str.cstr(), "-01:02:03");
}

test(TimeOffsetTest, toHourMinute) {
  int8_t hour;
  int8_t minute;
  TimeOffset offset;

  TimeOffset::forHourMinute(0, 15).toHourMinute(hour, minute);
  assertEqual(0, hour);
  assertEqual(15, minute);

  TimeOffset::forHourMinute(0, -15).toHourMinute(hour, minute);
  assertEqual(0, hour);
  assertEqual(-15, minute);

  TimeOffset::forHourMinute(-1, -15).toHourMinute(hour, minute);
  assertEqual(-1, hour);
  assertEqual(-15, minute);

  TimeOffset::forHourMinute(1, 15).toHourMinute(hour, minute);
  assertEqual(1, hour);
  assertEqual(15, minute);
}

test(TimeOffsetTest, toHourMinuteSecond) {
  int8_t hour;
  int8_t minute;
  int8_t second;

  TimeOffset::forHourMinuteSecond(0, 15, 1)
      .toHourMinuteSecond(hour, minute, second);
  assertEqual(0, hour);
  assertEqual(15, minute);
  assertEqual(1, second);

  TimeOffset::forHourMinuteSecond(0, -15, -2)
      .toHourMinuteSecond(hour, minute, second);
  assertEqual(0, hour);
  assertEqual(-15, minute);
  assertEqual(-2, second);

  TimeOffset::forHourMinuteSecond(-1, -15, -3)
      .toHourMinuteSecond(hour, minute, second);
  assertEqual(-1, hour);
  assertEqual(-15, minute);
  assertEqual(-3, second);

  TimeOffset::forHourMinuteSecond(1, 15, 4)
      .toHourMinuteSecond(hour, minute, second);
  assertEqual(1, hour);
  assertEqual(15, minute);
  assertEqual(4, second);
}

test(TimeOffsetTest, error) {
  TimeOffset offset;
  assertFalse(offset.isError());

  offset = TimeOffset::forError();
  assertTrue(offset.isError());
}

//---------------------------------------------------------------------------
// time_offset_mutation
//---------------------------------------------------------------------------

test(TimeOffsetMutationTest, increment15Minutes) {
  int8_t hour;
  int8_t minute;
  TimeOffset offset;

  offset = TimeOffset::forHourMinute(-16, 0);
  assertEqual(-960, offset.toMinutes());
  time_offset_mutation::increment15Minutes(offset);
  assertEqual(-945, offset.toMinutes());
  offset.toHourMinute(hour, minute);
  assertEqual(-15, hour);
  assertEqual(-45, minute);

  time_offset_mutation::increment15Minutes(offset);
  offset.toHourMinute(hour, minute);
  assertEqual(-15, hour);
  assertEqual(-30, minute);

  time_offset_mutation::increment15Minutes(offset);
  offset.toHourMinute(hour, minute);
  assertEqual(-15, hour);
  assertEqual(-15, minute);

  time_offset_mutation::increment15Minutes(offset);
  offset.toHourMinute(hour, minute);
  assertEqual(-15, hour);
  assertEqual(0, minute);

  offset = TimeOffset::forHourMinute(15, 45);
  time_offset_mutation::increment15Minutes(offset);
  offset.toHourMinute(hour, minute);
  assertEqual(16, hour);
  assertEqual(0, minute);

  time_offset_mutation::increment15Minutes(offset);
  offset.toHourMinute(hour, minute);
  assertEqual(-16, hour);
  assertEqual(0, minute);
}

//---------------------------------------------------------------------------

void setup() {
#if ! defined(EPOXY_DUINO)
  delay(1000); // wait to prevent garbage on SERIAL_PORT_MONITOR
#endif
  SERIAL_PORT_MONITOR.begin(115200);
  while (!SERIAL_PORT_MONITOR); // Leonardo/Micro
#if defined(EPOXY_DUINO)
  SERIAL_PORT_MONITOR.setLineModeUnix();
#endif
}

void loop() {
  aunit::TestRunner::run();
}
