#line 2 "DateTupleTest.ino"

/*
 * Unit tests for DateTupleTest.
 */

#include <AUnit.h>
#include <AceTime.h>

using namespace ace_time;
using namespace ace_time::extended;
using ace_time::internal::ZoneContext;

test(DateTuple, dateTupleOperatorLessThan) {
  assertTrue((
      DateTuple{2000, 1, 2, 3, ZoneContext::kSuffixW}
      < DateTuple{2000, 1, 2, 4, ZoneContext::kSuffixS}));
  assertTrue((
      DateTuple{2000, 1, 2, 3, ZoneContext::kSuffixW}
      < DateTuple{2000, 1, 3, 3, ZoneContext::kSuffixS}));
  assertTrue((
      DateTuple{2000, 1, 2, 3, ZoneContext::kSuffixW}
      < DateTuple{2000, 2, 2, 3, ZoneContext::kSuffixS}));
  assertTrue((
      DateTuple{2000, 1, 2, 3, ZoneContext::kSuffixW}
      < DateTuple{2001, 1, 2, 3, ZoneContext::kSuffixS}));
}

test(DateTuple, dateTupleOperatorEquals) {
  assertTrue((
      DateTuple{2000, 1, 2, 3, ZoneContext::kSuffixW}
      == DateTuple{2000, 1, 2, 3, ZoneContext::kSuffixW}));

  assertFalse((
      DateTuple{2000, 1, 2, 3, ZoneContext::kSuffixW}
      == DateTuple{2000, 1, 2, 3, ZoneContext::kSuffixS}));
  assertFalse((
      DateTuple{2000, 1, 2, 3, ZoneContext::kSuffixW}
      == DateTuple{2000, 1, 2, 4, ZoneContext::kSuffixW}));
  assertFalse((
      DateTuple{2000, 1, 2, 3, ZoneContext::kSuffixW}
      == DateTuple{2000, 1, 3, 3, ZoneContext::kSuffixW}));
  assertFalse((
      DateTuple{2000, 1, 2, 3, ZoneContext::kSuffixW}
      == DateTuple{2000, 2, 2, 3, ZoneContext::kSuffixW}));
  assertFalse((
      DateTuple{2000, 1, 2, 3, ZoneContext::kSuffixW}
      == DateTuple{2001, 1, 2, 3, ZoneContext::kSuffixW}));
}

test(DateTuple, normalizeDateTuple) {
  DateTuple dtp;

  dtp = {2000, 1, 1, 0, ZoneContext::kSuffixW};
  normalizeDateTuple(&dtp);
  assertTrue((dtp == DateTuple{2000, 1, 1, 0, ZoneContext::kSuffixW}));

  dtp = {2000, 1, 1, 15*95, ZoneContext::kSuffixW}; // 23:45
  normalizeDateTuple(&dtp);
  assertTrue((dtp == DateTuple{2000, 1, 1, 15*95, ZoneContext::kSuffixW}));

  dtp = {2000, 1, 1, 15*96, ZoneContext::kSuffixW}; // 24:00
  normalizeDateTuple(&dtp);
  assertTrue((dtp == DateTuple{2000, 1, 2, 0, ZoneContext::kSuffixW}));

  dtp = {2000, 1, 1, 15*97, ZoneContext::kSuffixW}; // 24:15
  normalizeDateTuple(&dtp);
  assertTrue((dtp == DateTuple{2000, 1, 2, 15, ZoneContext::kSuffixW}));

  dtp = {2000, 1, 1, -15*96, ZoneContext::kSuffixW}; // -24:00
  normalizeDateTuple(&dtp);
  assertTrue((dtp == DateTuple{1999, 12, 31, 0, ZoneContext::kSuffixW}));

  dtp = {2000, 1, 1, -15*97, ZoneContext::kSuffixW}; // -24:15
  normalizeDateTuple(&dtp);
  assertTrue((dtp == DateTuple{1999, 12, 31, -15, ZoneContext::kSuffixW}));
}

test(DateTuple, substractDateTuple) {
  DateTuple dta = {2000, 1, 1, 0, ZoneContext::kSuffixW}; // 2000-01-01 00:00
  DateTuple dtb = {2000, 1, 1, 1, ZoneContext::kSuffixW}; // 2000-01-01 00:01
  acetime_t diff = subtractDateTuple(dta, dtb);
  assertEqual(-60, diff);

  dta = {2000, 1, 1, 0, ZoneContext::kSuffixW}; // 2000-01-01 00:00
  dtb = {2000, 1, 2, 0, ZoneContext::kSuffixW}; // 2000-01-02 00:00
  diff = subtractDateTuple(dta, dtb);
  assertEqual((int32_t) -86400, diff);

  dta = {2000, 1, 1, 0, ZoneContext::kSuffixW}; // 2000-01-01 00:00
  dtb = {2000, 2, 1, 0, ZoneContext::kSuffixW}; // 2000-02-01 00:00
  diff = subtractDateTuple(dta, dtb);
  assertEqual((int32_t) -86400 * 31, diff); // January has 31 days

  dta = {2000, 2, 1, 0, ZoneContext::kSuffixW}; // 2000-02-01 00:00
  dtb = {2000, 3, 1, 0, ZoneContext::kSuffixW}; // 2000-03-01 00:00
  diff = subtractDateTuple(dta, dtb);
  assertEqual((int32_t) -86400 * 29, diff); // Feb 2000 is leap, 29 days
}

// Check that no overflow occurs even if DateTuple.year is more than 68 years
// greater than the Epoch::currentEpochYear(), which would normally cause the
// int32_t epochSeconds to overflow. Year 6000 is 4000 years beyond 2000, and
// 4000 years is a multiple of 400, so the Gregorian calendar in the year 6000
// should be identical to the year 2000, in particular, the leap years.
test(DateTuple, substractDateTuple_no_overflow) {
  DateTuple dta = {6000, 1, 1, 0, ZoneContext::kSuffixW}; // 6000-01-01 00:00
  DateTuple dtb = {6000, 1, 1, 1, ZoneContext::kSuffixW}; // 6000-01-01 00:01
  acetime_t diff = subtractDateTuple(dta, dtb);
  assertEqual(-60, diff);

  dta = {6000, 1, 1, 0, ZoneContext::kSuffixW}; // 6000-01-01 00:00
  dtb = {6000, 1, 2, 0, ZoneContext::kSuffixW}; // 6000-01-02 00:00
  diff = subtractDateTuple(dta, dtb);
  assertEqual((int32_t) -86400, diff);

  dta = {6000, 1, 1, 0, ZoneContext::kSuffixW}; // 6000-01-01 00:00
  dtb = {6000, 2, 1, 0, ZoneContext::kSuffixW}; // 6000-02-01 00:00
  diff = subtractDateTuple(dta, dtb);
  assertEqual((int32_t) -86400 * 31, diff); // January has 31 days

  dta = {6000, 2, 1, 0, ZoneContext::kSuffixW}; // 6000-02-01 00:00
  dtb = {6000, 3, 1, 0, ZoneContext::kSuffixW}; // 6000-03-01 00:00
  diff = subtractDateTuple(dta, dtb);
  assertEqual((int32_t) -86400 * 29, diff); // Feb 4000 is leap, 29 days
}

test(DateTuple, compareDateTupleFuzzy) {
  using ace_time::extended::CompareStatus;
  using ace_time::extended::DateTuple;

  assertEqual(
    (uint8_t) CompareStatus::kPrior,
    (uint8_t) compareDateTupleFuzzy(
      DateTuple{2000, 10, 1, 1, 0},
      DateTuple{2000, 12, 1, 1, 0},
      DateTuple{2002, 2, 1, 1, 0}));

  assertEqual(
    (uint8_t) CompareStatus::kWithinMatch,
    (uint8_t) compareDateTupleFuzzy(
      DateTuple{2000, 11, 1, 1, 0},
      DateTuple{2000, 12, 1, 1, 0},
      DateTuple{2002, 2, 1, 1, 0}));

  assertEqual(
    (uint8_t) CompareStatus::kWithinMatch,
    (uint8_t) compareDateTupleFuzzy(
      DateTuple{2000, 12, 1, 1, 0},
      DateTuple{2000, 12, 1, 1, 0},
      DateTuple{2002, 2, 1, 1, 0}));

  assertEqual(
    (uint8_t) CompareStatus::kWithinMatch,
    (uint8_t) compareDateTupleFuzzy(
      DateTuple{2002, 2, 1, 1, 0},
      DateTuple{2000, 12, 1, 1, 0},
      DateTuple{2002, 2, 1, 1, 0}));

  assertEqual(
    (uint8_t) CompareStatus::kWithinMatch,
    (uint8_t) compareDateTupleFuzzy(
      DateTuple{2002, 3, 1, 1, 0},
      DateTuple{2000, 12, 1, 1, 0},
      DateTuple{2002, 2, 1, 1, 0}));

  assertEqual(
    (uint8_t) CompareStatus::kFarFuture,
    (uint8_t) compareDateTupleFuzzy(
      DateTuple{2002, 4, 1, 1, 0},
      DateTuple{2000, 12, 1, 1, 0},
      DateTuple{2002, 2, 1, 1, 0}));

  // Verify dates whose delta months is greater than 32767. In
  // other words, delta years is greater than 2730.
  assertEqual(
    (uint8_t) CompareStatus::kFarFuture,
    (uint8_t) compareDateTupleFuzzy(
      DateTuple{5000, 4, 1, 1, 0},
      DateTuple{2000, 12, 1, 1, 0},
      DateTuple{2002, 2, 1, 1, 0}));
  assertEqual(
    (uint8_t) CompareStatus::kPrior,
    (uint8_t) compareDateTupleFuzzy(
      DateTuple{1000, 4, 1, 1, 0},
      DateTuple{4000, 12, 1, 1, 0},
      DateTuple{4002, 2, 1, 1, 0}));
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
