#line 2 "PlainTimeTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace ace_time;

//---------------------------------------------------------------------------
// PlainTime
//---------------------------------------------------------------------------

test(PlainTimeTest, accessors_mutators) {
  // accessors
  PlainTime pt = PlainTime::forComponents(1, 2, 3);
  assertEqual(1, pt.hour());
  assertEqual(2, pt.minute());
  assertEqual(3, pt.second());

  // mutators
  pt.hour(11);
  pt.minute(12);
  pt.second(13);
  assertEqual(11, pt.hour());
  assertEqual(12, pt.minute());
  assertEqual(13, pt.second());
}

test(PlainTimeTest, isError) {
  assertFalse(PlainTime::forComponents(0, 0, 0).isError());
  assertFalse(PlainTime::forComponents(0, 59, 0).isError());
  assertFalse(PlainTime::forComponents(0, 59, 59).isError());
  assertFalse(PlainTime::forComponents(23, 59, 59).isError());
  assertFalse(PlainTime::forComponents(24, 0, 0).isError());

  assertTrue(PlainTime::forComponents(24, 0, 1).isError());
  assertTrue(PlainTime::forComponents(25, 0, 0).isError());
  assertTrue(PlainTime::forComponents(0, 60, 0).isError());
  assertTrue(PlainTime::forComponents(0, 0, 60).isError());
}

test(PlainTimeTest, forError) {
  PlainTime pt = PlainTime::forError();
  assertTrue(pt.isError());
  assertEqual(PlainTime::kInvalidSeconds, pt.toSeconds());

  pt = PlainTime::forSeconds(PlainTime::kInvalidSeconds);
  assertTrue(pt.isError());
}

test(PlainTimeTest, toAndFromSeconds) {
  PlainTime pt;

  pt = PlainTime::forSeconds(0);
  assertTrue(pt == PlainTime::forComponents(0, 0, 0));
  assertEqual((acetime_t) 0, pt.toSeconds());

  pt = PlainTime::forSeconds(3662);
  assertTrue(pt == PlainTime::forComponents(1, 1, 2));
  assertEqual((acetime_t) 3662, pt.toSeconds());

  pt = PlainTime::forSeconds(86399);
  assertTrue(pt == PlainTime::forComponents(23, 59, 59));
  assertEqual((acetime_t) 86399, pt.toSeconds());
}

test(PlainTimeTest, compareTo) {
  PlainTime a, b;

  a = PlainTime::forComponents(0, 1, 1);
  b = PlainTime::forComponents(0, 1, 1);
  assertEqual(a.compareTo(b), 0);
  assertTrue(a == b);
  assertFalse(a != b);

  a = PlainTime::forComponents(0, 1, 1);
  b = PlainTime::forComponents(0, 1, 2);
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);

  a = PlainTime::forComponents(0, 1, 1);
  b = PlainTime::forComponents(0, 2, 1);
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);

  a = PlainTime::forComponents(0, 1, 1);
  b = PlainTime::forComponents(1, 1, 1);
  assertLess(a.compareTo(b), 0);
  assertMore(b.compareTo(a), 0);
  assertTrue(a != b);
}

test(PlainTimeTest, forTimeString) {
  PlainTime pt;
  pt = PlainTime::forTimeString("00:00:00");
  assertTrue(pt == PlainTime::forComponents(0, 0, 0));

  pt = PlainTime::forTimeString("01:02:03");
  assertTrue(pt == PlainTime::forComponents(1, 2, 3));
}

test(PlainTimeTest, fortimeString_invalid) {
  PlainTime pt = PlainTime::forTimeString("01:02");
  assertTrue(pt.isError());
}

//---------------------------------------------------------------------------

void setup() {
#if ! defined(EPOXY_DUINO)
  delay(1000); // wait to prevent garbage SERIAL_PORT_MONITOR
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
