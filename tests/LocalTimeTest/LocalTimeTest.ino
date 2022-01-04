#line 2 "LocalTimeTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace ace_time;

//---------------------------------------------------------------------------
// LocalTime
//---------------------------------------------------------------------------

test(LocalTimeTest, accessors) {
  LocalTime lt = LocalTime::forComponents(1, 2, 3);
  assertEqual(1, lt.hour());
  assertEqual(2, lt.minute());
  assertEqual(3, lt.second());
}

test(LocalTimeTest, isError) {
  assertFalse(LocalTime::forComponents(0, 0, 0).isError());
  assertFalse(LocalTime::forComponents(0, 59, 0).isError());
  assertFalse(LocalTime::forComponents(0, 59, 59).isError());
  assertFalse(LocalTime::forComponents(23, 59, 59).isError());
  assertFalse(LocalTime::forComponents(24, 0, 0).isError());

  assertTrue(LocalTime::forComponents(24, 0, 1).isError());
  assertTrue(LocalTime::forComponents(25, 0, 0).isError());
  assertTrue(LocalTime::forComponents(0, 60, 0).isError());
  assertTrue(LocalTime::forComponents(0, 0, 60).isError());
}

test(LocalTimeTest, forError) {
  LocalTime lt = LocalTime::forError();
  assertTrue(lt.isError());
  assertEqual(LocalTime::kInvalidSeconds, lt.toSeconds());

  lt = LocalTime::forSeconds(LocalTime::kInvalidSeconds);
  assertTrue(lt.isError());
}

test(LocalTimeTest, toAndFromSeconds) {
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

test(LocalTimeTest, compareTo) {
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

test(LocalTimeTest, forTimeString) {
  LocalTime lt;
  lt = LocalTime::forTimeString("00:00:00");
  assertTrue(lt == LocalTime::forComponents(0, 0, 0));

  lt = LocalTime::forTimeString("01:02:03");
  assertTrue(lt == LocalTime::forComponents(1, 2, 3));
}

test(LocalTimeTest, fortimeString_invalid) {
  LocalTime lt = LocalTime::forTimeString("01:02");
  assertTrue(lt.isError());
}

//---------------------------------------------------------------------------

void setup() {
#if ! defined(EPOXY_DUINO)
  delay(1000); // wait to prevent garbage SERIAL_PORT_MONITOR
#endif
  SERIAL_PORT_MONITOR.begin(115200);
  while (!SERIAL_PORT_MONITOR); // Leonardo/Micro
}

void loop() {
  aunit::TestRunner::run();
}
