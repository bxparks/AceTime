#line 2 "DateTupleTest.ino"

/*
 * Unit tests for DateTupleTest.
 */

#include <AUnit.h>
#include <AceTime.h>

using namespace ace_time;
using namespace ace_time::extended;
using ace_time::extended::Info;

static const auto kSuffixW = Info::ZoneContext::kSuffixW;
static const auto kSuffixS = Info::ZoneContext::kSuffixS;
static const auto kSuffixU = Info::ZoneContext::kSuffixU;

test(DateTuple, dateTupleOperatorLessThan) {
  assertTrue((
      DateTuple{2000, 1, 2, 3, kSuffixW}
      < DateTuple{2000, 1, 2, 4, kSuffixS}));
  assertTrue((
      DateTuple{2000, 1, 2, 3, kSuffixW}
      < DateTuple{2000, 1, 3, 3, kSuffixS}));
  assertTrue((
      DateTuple{2000, 1, 2, 3, kSuffixW}
      < DateTuple{2000, 2, 2, 3, kSuffixS}));
  assertTrue((
      DateTuple{2000, 1, 2, 3, kSuffixW}
      < DateTuple{2001, 1, 2, 3, kSuffixS}));
}

test(DateTuple, dateTupleOperatorEquals) {
  assertTrue((
      DateTuple{2000, 1, 2, 3, kSuffixW}
      == DateTuple{2000, 1, 2, 3, kSuffixW}));

  assertFalse((
      DateTuple{2000, 1, 2, 3, kSuffixW}
      == DateTuple{2000, 1, 2, 3, kSuffixS}));
  assertFalse((
      DateTuple{2000, 1, 2, 3, kSuffixW}
      == DateTuple{2000, 1, 2, 4, kSuffixW}));
  assertFalse((
      DateTuple{2000, 1, 2, 3, kSuffixW}
      == DateTuple{2000, 1, 3, 3, kSuffixW}));
  assertFalse((
      DateTuple{2000, 1, 2, 3, kSuffixW}
      == DateTuple{2000, 2, 2, 3, kSuffixW}));
  assertFalse((
      DateTuple{2000, 1, 2, 3, kSuffixW}
      == DateTuple{2001, 1, 2, 3, kSuffixW}));
}

test(DateTuple, normalizeDateTuple) {
  DateTuple dtp;

  dtp = {2000, 1, 1, 0, kSuffixW};
  normalizeDateTuple(&dtp);
  assertTrue((dtp == DateTuple{2000, 1, 1, 0, kSuffixW}));

  dtp = {2000, 1, 1, (23*60+45)*60, kSuffixW}; // 23:45
  normalizeDateTuple(&dtp);
  assertTrue((dtp == DateTuple{2000, 1, 1, (23*60+45)*60, kSuffixW}));

  dtp = {2000, 1, 1, 24*60*60, kSuffixW}; // 24:00
  normalizeDateTuple(&dtp);
  assertTrue((dtp == DateTuple{2000, 1, 2, 0, kSuffixW}));

  dtp = {2000, 1, 1, (24*60+15)*60, kSuffixW}; // 24:15
  normalizeDateTuple(&dtp);
  assertTrue((dtp == DateTuple{2000, 1, 2, 15*60, kSuffixW}));

  dtp = {2000, 1, 1, -24*60*60, kSuffixW}; // -24:00
  normalizeDateTuple(&dtp);
  assertTrue((dtp == DateTuple{1999, 12, 31, 0, kSuffixW}));

  dtp = {2000, 1, 1, -(24*60+15)*60, kSuffixW}; // -24:15
  normalizeDateTuple(&dtp);
  assertTrue((dtp == DateTuple{1999, 12, 31, -15*60, kSuffixW}));
}

test(DateTuple, expandDateTuple) {
  DateTuple ttw;
  DateTuple tts;
  DateTuple ttu;
  int32_t offsetSeconds = 2*60*60;
  int32_t deltaSeconds = 1*60*60;

  DateTuple tt = {2000, 1, 30, 4*60*60, kSuffixW}; // 04:00
  expandDateTuple(&tt, offsetSeconds, deltaSeconds, &ttw, &tts, &ttu);
  assertTrue((ttw == DateTuple{2000, 1, 30, 4*60*60, kSuffixW}));
  assertTrue((tts == DateTuple{2000, 1, 30, 3*60*60, kSuffixS}));
  assertTrue((ttu == DateTuple{2000, 1, 30, 1*60*60, kSuffixU}));

  tt = {2000, 1, 30, 3*60*60, kSuffixS};
  expandDateTuple(&tt, offsetSeconds, deltaSeconds, &ttw, &tts, &ttu);
  assertTrue((ttw == DateTuple{2000, 1, 30, 4*60*60, kSuffixW}));
  assertTrue((tts == DateTuple{2000, 1, 30, 3*60*60, kSuffixS}));
  assertTrue((ttu == DateTuple{2000, 1, 30, 1*60*60, kSuffixU}));

  tt = {2000, 1, 30, 1*60*60, kSuffixU};
  expandDateTuple(&tt, offsetSeconds, deltaSeconds, &ttw, &tts, &ttu);
  assertTrue((ttw == DateTuple{2000, 1, 30, 4*60*60, kSuffixW}));
  assertTrue((tts == DateTuple{2000, 1, 30, 3*60*60, kSuffixS}));
  assertTrue((ttu == DateTuple{2000, 1, 30, 1*60*60, kSuffixU}));
}

// Validate fix for bug that performed a cast to (int16_t) minutes, instead of
// (int32_t) seconds.
test(DateTuple, expandDateTuple_largeOffset) {
  DateTuple ttw;
  DateTuple tts;
  DateTuple ttu;

  int32_t offsetSeconds = 23*60*60; // 82800
  int32_t deltaSeconds = 23*60*60; // 82800
  DateTuple tt = {2000, 1, 30, 23*60*60, kSuffixS}; // 23:00s
  expandDateTuple(&tt, offsetSeconds, deltaSeconds, &ttw, &tts, &ttu);
  assertTrue((ttw == DateTuple{2000, 1, 31, 22*60*60, kSuffixW}));
  assertTrue((tts == DateTuple{2000, 1, 30, 23*60*60, kSuffixS}));
  assertTrue((ttu == DateTuple{2000, 1, 30, 0*60*60, kSuffixU}));

  offsetSeconds = -23*60*60; // 82800
  deltaSeconds = -23*60*60; // 7200
  tt = {2000, 1, 31, 1*60*60, kSuffixS}; // 01:00s
  expandDateTuple(&tt, offsetSeconds, deltaSeconds, &ttw, &tts, &ttu);
  assertTrue((ttw == DateTuple{2000, 1, 31, -22*60*60, kSuffixW}));
  assertTrue((tts == DateTuple{2000, 1, 31, 1*60*60, kSuffixS}));
  assertTrue((ttu == DateTuple{2000, 2, 1, 0*60*60, kSuffixU}));

  offsetSeconds = 23*60*60; // 82800
  deltaSeconds = 1*60*60; // 82800
  tt = {2000, 1, 30, 23*60*60, kSuffixU}; // 23:00u
  expandDateTuple(&tt, offsetSeconds, deltaSeconds, &ttw, &tts, &ttu);
  assertTrue((ttw == DateTuple{2000, 1, 31, 23*60*60, kSuffixW}));
  assertTrue((tts == DateTuple{2000, 1, 31, 22*60*60, kSuffixS}));
  assertTrue((ttu == DateTuple{2000, 1, 30, 23*60*60, kSuffixU}));

  offsetSeconds = -23*60*60; // 82800
  deltaSeconds = -1*60*60; // 7200
  tt = {2000, 1, 31, 1*60*60, kSuffixU}; // 01:00u
  expandDateTuple(&tt, offsetSeconds, deltaSeconds, &ttw, &tts, &ttu);
  assertTrue((ttw == DateTuple{2000, 1, 31, -23*60*60, kSuffixW}));
  assertTrue((tts == DateTuple{2000, 1, 31, -22*60*60, kSuffixS}));
  assertTrue((ttu == DateTuple{2000, 1, 31, 1*60*60, kSuffixU}));
}

test(DateTuple, substractDateTuple) {
  DateTuple dta = {2000, 1, 1, 0, kSuffixW}; // 2000-01-01 00:00
  DateTuple dtb = {2000, 1, 1, 60, kSuffixW}; // 2000-01-01 00:01
  acetime_t diff = subtractDateTuple(dta, dtb);
  assertEqual((acetime_t) -60, diff);

  dta = {2000, 1, 1, 0, kSuffixW}; // 2000-01-01 00:00
  dtb = {2000, 1, 2, 0, kSuffixW}; // 2000-01-02 00:00
  diff = subtractDateTuple(dta, dtb);
  assertEqual((acetime_t) -86400, diff);

  dta = {2000, 1, 1, 0, kSuffixW}; // 2000-01-01 00:00
  dtb = {2000, 2, 1, 0, kSuffixW}; // 2000-02-01 00:00
  diff = subtractDateTuple(dta, dtb);
  assertEqual((acetime_t) -86400 * 31, diff); // January has 31 days

  dta = {2000, 2, 1, 0, kSuffixW}; // 2000-02-01 00:00
  dtb = {2000, 3, 1, 0, kSuffixW}; // 2000-03-01 00:00
  diff = subtractDateTuple(dta, dtb);
  assertEqual((acetime_t) -86400 * 29, diff); // Feb 2000 is leap, 29 days
}

// Check that no overflow occurs even if DateTuple.year is more than 68 years
// greater than the Epoch::currentEpochYear(), which would normally cause the
// int32_t epochSeconds to overflow. Year 6000 is 4000 years beyond 2000, and
// 4000 years is a multiple of 400, so the Gregorian calendar in the year 6000
// should be identical to the year 2000, in particular, the leap years.
test(DateTuple, substractDateTuple_no_overflow) {
  DateTuple dta = {6000, 1, 1, 0, kSuffixW}; // 6000-01-01 00:00
  DateTuple dtb = {6000, 1, 1, 60, kSuffixW}; // 6000-01-01 00:01
  acetime_t diff = subtractDateTuple(dta, dtb);
  assertEqual((acetime_t) -60, diff);

  dta = {6000, 1, 1, 0, kSuffixW}; // 6000-01-01 00:00
  dtb = {6000, 1, 2, 0, kSuffixW}; // 6000-01-02 00:00
  diff = subtractDateTuple(dta, dtb);
  assertEqual((acetime_t) -86400, diff);

  dta = {6000, 1, 1, 0, kSuffixW}; // 6000-01-01 00:00
  dtb = {6000, 2, 1, 0, kSuffixW}; // 6000-02-01 00:00
  diff = subtractDateTuple(dta, dtb);
  assertEqual((acetime_t) -86400 * 31, diff); // January has 31 days

  dta = {6000, 2, 1, 0, kSuffixW}; // 6000-02-01 00:00
  dtb = {6000, 3, 1, 0, kSuffixW}; // 6000-03-01 00:00
  diff = subtractDateTuple(dta, dtb);
  assertEqual((acetime_t) -86400 * 29, diff); // Feb 4000 is leap, 29 days
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
