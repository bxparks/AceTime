#line 2 "TimePeriodTest.ino"

#include <AUnit.h>
#include <AceTime.h>
#include <AceCommon.h>

using namespace ace_time;
using ace_common::PrintStr;

//---------------------------------------------------------------------------
// TimePeriod
//---------------------------------------------------------------------------

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
  time_period_mutation::incrementHour(a);
  TimePeriod expected(4, 2, 1, 1);
  assertTrue(expected == a);

  a = TimePeriod(23, 2, 1, -1);
  time_period_mutation::incrementHour(a);
  expected = TimePeriod(0, 2, 1, -1);
  assertTrue(expected == a);
}

test(TimePeriodTest, incrementMinute) {
  TimePeriod a(3, 2, 1, 1);
  time_period_mutation::incrementMinute(a);
  TimePeriod expected(3, 3, 1, 1);
  assertTrue(expected == a);

  a = TimePeriod(3, 59, 1, 1);
  time_period_mutation::incrementMinute(a);
  expected = TimePeriod(3, 0, 1, 1);
  assertTrue(expected == a);
}

test(TimePeriodTest, negate) {
  TimePeriod a(3, 2, 1, 1);
  assertEqual((int8_t) 1, a.sign());
  time_period_mutation::negate(a);
  assertEqual((int8_t) -1, a.sign());
}

test(TimePeriodTest, forError_default) {
  TimePeriod period = TimePeriod::forError();
  assertTrue(period.isError());
  assertEqual(TimePeriod::kInvalidPeriodSeconds, period.toSeconds());
  assertEqual(period.sign(), 0);

  PrintStr<16> message;
  period.printTo(message);
  assertEqual(F("<Error>"), message.cstr());
}

test(TimePeriodTest, forError_overflow) {
  TimePeriod period = TimePeriod::forError(1);
  assertTrue(period.isError());
  assertEqual(TimePeriod::kInvalidPeriodSeconds, period.toSeconds());
  assertEqual(period.sign(), 1);

  PrintStr<16> message;
  period.printTo(message);
  assertEqual(F("<+Inf>"), message.cstr());
}

test(TimePeriodTest, forError_underflow) {
  TimePeriod period = TimePeriod::forError(-1);
  assertTrue(period.isError());
  assertEqual(TimePeriod::kInvalidPeriodSeconds, period.toSeconds());
  assertEqual(period.sign(), -1);

  PrintStr<16> message;
  period.printTo(message);
  assertEqual(F("<-Inf>"), message.cstr());
}

test(TimePeriodTest, fromSeconds_max) {
  TimePeriod largest(TimePeriod::kMaxPeriodSeconds);
  assertFalse(largest.isError());
  assertEqual(TimePeriod::kMaxPeriodSeconds, largest.toSeconds());
}

test(TimePeriodTest, fromSeconds_min) {
  TimePeriod smallest(-TimePeriod::kMaxPeriodSeconds);
  assertFalse(smallest.isError());
  assertEqual(-TimePeriod::kMaxPeriodSeconds, smallest.toSeconds());
}

test(TimePeriodTest, fromSeconds_invalid) {
  TimePeriod invalid(TimePeriod::kInvalidPeriodSeconds);
  assertTrue(invalid.isError());
  assertEqual(invalid.sign(), 0);
}

test(TimePeriodTest, fromSeconds_tooLarge) {
  TimePeriod tooLarge(TimePeriod::kMaxPeriodSeconds + 1);
  assertTrue(tooLarge.isError());
  assertEqual(tooLarge.sign(), 1);
}

test(TimePeriodTest, fromSeconds_tooSmall) {
  TimePeriod tooSmall(-TimePeriod::kMaxPeriodSeconds - 1);
  assertTrue(tooSmall.isError());
  assertEqual(tooSmall.sign(), -1);
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
