#line 2 "TimeZoneTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time;
using namespace ace_time::common;

// --------------------------------------------------------------------------
// ManualZoneSpec
// --------------------------------------------------------------------------

test(ManualZoneSpecTest, accessors) {
  ManualZoneSpec pstSpec(
      UtcOffset::forHour(-8), UtcOffset::forHour(1), "PST", "PDT");

  assertEqual(-8*60, pstSpec.getUtcOffset().toMinutes());
  assertEqual("PST", pstSpec.getAbbrev());
  assertEqual(0, pstSpec.getDeltaOffset().toMinutes());

  pstSpec.isDst(true);

  assertEqual(-7*60, pstSpec.getUtcOffset().toMinutes());
  assertEqual("PDT", pstSpec.getAbbrev());
  assertEqual(1*60, pstSpec.getDeltaOffset().toMinutes());
}

test(ManualZoneSpecTest, copyConstructor) {
  ManualZoneSpec a(
      UtcOffset::forHour(-8), UtcOffset::forHour(1), "PST", "PDT");
  ManualZoneSpec b(a);

  assertEqual(a.isDst(), b.isDst());
  assertEqual(a.stdOffset().toMinutes(), b.stdOffset().toMinutes());
  assertEqual(a.stdAbbrev(), b.stdAbbrev());
  assertEqual(a.deltaOffset().toMinutes(), b.deltaOffset().toMinutes());
  assertEqual(a.dstAbbrev(), b.dstAbbrev());

  b.isDst(true);
  assertNotEqual(a.isDst(), b.isDst());
}

test(ManualZoneSpecTest, operatorEqualEqual) {
  ManualZoneSpec a(
      UtcOffset::forHour(1), UtcOffset::forHour(1), "a", "b");
  ManualZoneSpec b(
      UtcOffset::forHour(2), UtcOffset::forHour(1), "a", "b");
  ManualZoneSpec c(
      UtcOffset::forHour(1), UtcOffset::forHour(1), "A", "b");
  ManualZoneSpec d(
      UtcOffset::forHour(1), UtcOffset::forHour(2), "a", "b");
  ManualZoneSpec e(
      UtcOffset::forHour(1), UtcOffset::forHour(1), "a", "B");

  assertTrue(a != b);
  assertTrue(a != c);
  assertTrue(a != d);
  assertTrue(a != e);

  ManualZoneSpec aa(a);
  assertTrue(a == aa);

  aa.isDst(true);
  assertTrue(a != aa);
}

// --------------------------------------------------------------------------
// AutoZoneSpec
// --------------------------------------------------------------------------

test(AutoZoneSpecTest, operatorEqualEqual) {
  AutoZoneSpec a(&zonedb::kZoneLos_Angeles);
  AutoZoneSpec b(&zonedb::kZoneDarwin);
  assertTrue(a != b);
}

test(AutoZoneSpecTest, calcStartDayOfMonth) {
  // 2018-11, Sun>=1
  assertEqual(4, AutoZoneSpec::calcStartDayOfMonth(
      2018, 11, LocalDate::kSunday, 1));

  // 2018-11, lastSun
  assertEqual(25, AutoZoneSpec::calcStartDayOfMonth(
      2018, 11, LocalDate::kSunday, 0));

  // 2018-03, Thu>=9
  assertEqual(15, AutoZoneSpec::calcStartDayOfMonth(
      2018, 3, LocalDate::kThursday, 9));

  // 2018-03-30
  assertEqual(30, AutoZoneSpec::calcStartDayOfMonth(2018, 3, 0, 30));
}

test(AutoZoneSpecTest, calcRuleOffsetCode) {
  assertEqual(0, AutoZoneSpec::calcRuleOffsetCode(1, 2, 'u'));
  assertEqual(1, AutoZoneSpec::calcRuleOffsetCode(1, 2, 'w'));
  assertEqual(2, AutoZoneSpec::calcRuleOffsetCode(1, 2, 's'));
}

test(AutoZoneSpecTest, init_primitives) {
  AutoZoneSpec zoneSpec(&zonedb::kZoneLos_Angeles);
  zoneSpec.mYear = 2001;
  zoneSpec.mNumMatches = 0;

  zoneSpec.addRulePriorToYear(2001);
  assertEqual(0, zoneSpec.mNumMatches);
  assertEqual(-32, zoneSpec.mPreviousMatch.era->offsetCode);
  assertEqual("P%T", zoneSpec.mPreviousMatch.era->format);
  assertEqual(1967-2000, zoneSpec.mPreviousMatch.rule->fromYearTiny);
  assertEqual(2006-2000, zoneSpec.mPreviousMatch.rule->toYearTiny);
  assertEqual(10, zoneSpec.mPreviousMatch.rule->inMonth);

  zoneSpec.addRulesForYear(2001);
  assertEqual(2, zoneSpec.mNumMatches);

  assertEqual(-32, zoneSpec.mMatches[0].era->offsetCode);
  assertEqual("P%T", zoneSpec.mMatches[0].era->format);
  assertEqual(1987-2000, zoneSpec.mMatches[0].rule->fromYearTiny);
  assertEqual(2006-2000, zoneSpec.mMatches[0].rule->toYearTiny);
  assertEqual(4, zoneSpec.mMatches[0].rule->inMonth);

  assertEqual(-32, zoneSpec.mMatches[1].era->offsetCode);
  assertEqual("P%T", zoneSpec.mMatches[1].era->format);
  assertEqual(1967-2000, zoneSpec.mMatches[1].rule->fromYearTiny);
  assertEqual(2006-2000, zoneSpec.mMatches[1].rule->toYearTiny);
  assertEqual(10, zoneSpec.mMatches[1].rule->inMonth);

  zoneSpec.calcTransitions();
  assertEqual((acetime_t) 0, zoneSpec.mPreviousMatch.startEpochSeconds);
  assertEqual(-32, zoneSpec.mPreviousMatch.offsetCode);

  // t >= 2001-04-01 02:00 UTC-08:00 Sunday goes to PDT
  assertEqual(-28, zoneSpec.mMatches[0].offsetCode);
  assertEqual((acetime_t) 39434400, zoneSpec.mMatches[0].startEpochSeconds);

  // t >= 2001-10-28 02:00 UTC-07:00 Sunday goes to PST
  assertEqual(-32, zoneSpec.mMatches[1].offsetCode);
  assertEqual((acetime_t) 57574800, zoneSpec.mMatches[1].startEpochSeconds);
}

test(AutoZoneSpecTest, init) {
  AutoZoneSpec zoneSpec(&zonedb::kZoneLos_Angeles);
  LocalDate ld = LocalDate::forComponents(2018, 1, 1); // 2018-01-01
  zoneSpec.init(ld);

  assertEqual(2, zoneSpec.mNumMatches);

  assertEqual(-32, zoneSpec.mPreviousMatch.era->offsetCode);
  assertEqual("P%T", zoneSpec.mPreviousMatch.era->format);
  assertEqual(2007-2000, zoneSpec.mPreviousMatch.rule->fromYearTiny);
  assertEqual(ZoneRule::kMaxYearTiny,
      zoneSpec.mPreviousMatch.rule->toYearTiny);
  assertEqual(11, zoneSpec.mPreviousMatch.rule->inMonth);

  assertEqual(-32, zoneSpec.mMatches[0].era->offsetCode);
  assertEqual("P%T", zoneSpec.mMatches[0].era->format);
  assertEqual(2007-2000, zoneSpec.mMatches[0].rule->fromYearTiny);
  assertEqual(ZoneRule::kMaxYearTiny, zoneSpec.mMatches[0].rule->toYearTiny);
  assertEqual(3, zoneSpec.mMatches[0].rule->inMonth);

  assertEqual(-32, zoneSpec.mMatches[1].era->offsetCode);
  assertEqual("P%T", zoneSpec.mMatches[1].era->format);
  assertEqual(2007-2000, zoneSpec.mMatches[1].rule->fromYearTiny);
  assertEqual(ZoneRule::kMaxYearTiny, zoneSpec.mMatches[1].rule->toYearTiny);
  assertEqual(11, zoneSpec.mMatches[1].rule->inMonth);

  assertEqual((acetime_t) 0, zoneSpec.mPreviousMatch.startEpochSeconds);
  assertEqual(-32, zoneSpec.mPreviousMatch.offsetCode);

  // t >= 2018-03-11 02:00 UTC-08:00 Sunday goes to PDT
  assertEqual(-28, zoneSpec.mMatches[0].offsetCode);
  assertEqual((acetime_t) 574077600, zoneSpec.mMatches[0].startEpochSeconds);

  // t >= 2018-11-04 02:00 UTC-07:00 Sunday goes to PST
  assertEqual(-32, zoneSpec.mMatches[1].offsetCode);
  assertEqual((acetime_t) 594637200, zoneSpec.mMatches[1].startEpochSeconds);
}

// zoneInfo == nullptr means UTC
test(AutoZoneSpecTest, nullptr) {
  AutoZoneSpec zoneSpec(nullptr);
  assertEqual(0, zoneSpec.getUtcOffset(0).toMinutes());
  assertEqual("UTC", zoneSpec.getAbbrev(0));
  assertFalse(zoneSpec.getDeltaOffset(0).isDst());
}

test(AutoZoneSpecTest, copyConstructorAssignmentOperator) {
  OffsetDateTime dt = OffsetDateTime::forComponents(2018, 3, 11, 1, 59, 59,
      UtcOffset::forHour(-8));
  acetime_t epochSeconds = dt.toEpochSeconds();

  AutoZoneSpec m1(nullptr);
  assertEqual(0, m1.getUtcOffset(0).toMinutes());

  AutoZoneSpec m2(&zonedb::kZoneLos_Angeles);
  assertEqual(-8*60, m2.getUtcOffset(epochSeconds).toMinutes());

  m1 = m2;
  assertEqual(-8*60, m1.getUtcOffset(0).toMinutes());

  AutoZoneSpec m3(m2);
  assertEqual(-8*60, m1.getUtcOffset(0).toMinutes());
}

// https://www.timeanddate.com/time/zone/usa/los-angeles
test(AutoZoneSpecTest, kZoneLos_Angeles) {
  AutoZoneSpec zoneSpec(&zonedb::kZoneLos_Angeles);
  OffsetDateTime dt;
  acetime_t epochSeconds;

  dt = OffsetDateTime::forComponents(2018, 3, 11, 1, 59, 59,
      UtcOffset::forHour(-8));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-8*60, zoneSpec.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("PST", zoneSpec.getAbbrev(epochSeconds));
  assertFalse(zoneSpec.getDeltaOffset(epochSeconds).isDst());

  dt = OffsetDateTime::forComponents(2018, 3, 11, 2, 0, 0,
      UtcOffset::forHour(-8));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-7*60, zoneSpec.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("PDT", zoneSpec.getAbbrev(epochSeconds));
  assertTrue(zoneSpec.getDeltaOffset(epochSeconds).isDst());

  dt = OffsetDateTime::forComponents(2018, 11, 4, 1, 0, 0,
      UtcOffset::forHour(-7));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-7*60, zoneSpec.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("PDT", zoneSpec.getAbbrev(epochSeconds));
  assertTrue(zoneSpec.getDeltaOffset(epochSeconds).isDst());

  dt = OffsetDateTime::forComponents(2018, 11, 4, 1, 59, 59,
      UtcOffset::forHour(-7));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-7*60, zoneSpec.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("PDT", zoneSpec.getAbbrev(epochSeconds));
  assertTrue(zoneSpec.getDeltaOffset(epochSeconds).isDst());

  dt = OffsetDateTime::forComponents(2018, 11, 4, 2, 0, 0,
      UtcOffset::forHour(-7));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-8*60, zoneSpec.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("PST", zoneSpec.getAbbrev(epochSeconds));
  assertFalse(zoneSpec.getDeltaOffset(epochSeconds).isDst());
}

// https://www.timeanddate.com/time/zone/australia/sydney
test(AutoZoneSpecTest, kZoneSydney) {
  AutoZoneSpec zoneSpec(&zonedb::kZoneSydney);
  OffsetDateTime dt;
  acetime_t epochSeconds;

  dt = OffsetDateTime::forComponents(2007, 3, 25, 2, 59, 59,
      UtcOffset::forHour(11));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(11*60, zoneSpec.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("AEDT", zoneSpec.getAbbrev(epochSeconds));
  assertTrue(zoneSpec.getDeltaOffset(epochSeconds).isDst());

  dt = OffsetDateTime::forComponents(2007, 3, 25, 3, 0, 0,
      UtcOffset::forHour(11));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(10*60, zoneSpec.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("AEST", zoneSpec.getAbbrev(epochSeconds));
  assertFalse(zoneSpec.getDeltaOffset(epochSeconds).isDst());

  dt = OffsetDateTime::forComponents(2007, 10, 28, 1, 59, 59,
      UtcOffset::forHour(10));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(10*60, zoneSpec.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("AEST", zoneSpec.getAbbrev(epochSeconds));
  assertFalse(zoneSpec.getDeltaOffset(epochSeconds).isDst());

  dt = OffsetDateTime::forComponents(2007, 10, 28, 2, 0, 0,
      UtcOffset::forHour(10));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(11*60, zoneSpec.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("AEDT", zoneSpec.getAbbrev(epochSeconds));
  assertTrue(zoneSpec.getDeltaOffset(epochSeconds).isDst());
}

// https://www.timeanddate.com/time/zone/south-africa/johannesburg
// No DST changes at all.
test(AutoZoneSpecTest, kZoneJohannesburg) {
  AutoZoneSpec zoneSpec(&zonedb::kZoneJohannesburg);
  OffsetDateTime dt;
  acetime_t epochSeconds;

  dt = OffsetDateTime::forComponents(2018, 1, 1, 0, 0, 0,
      UtcOffset::forHour(2));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(2*60, zoneSpec.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("SAST", zoneSpec.getAbbrev(epochSeconds));
  assertFalse(zoneSpec.getDeltaOffset(epochSeconds).isDst());
}

// https://www.timeanddate.com/time/zone/australia/darwin
// No DST changes since 1944. Uses the last transition which occurred in March
// 1944.
test(AutoZoneSpecTest, kZoneDarwin) {
  AutoZoneSpec zoneSpec(&zonedb::kZoneDarwin);
  OffsetDateTime dt;
  acetime_t epochSeconds;

  dt = OffsetDateTime::forComponents(2018, 1, 1, 0, 0, 0,
      UtcOffset::forHourMinute(1, 9, 30));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(9*60+30, zoneSpec.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("ACST", zoneSpec.getAbbrev(epochSeconds));
  assertFalse(zoneSpec.getDeltaOffset(epochSeconds).isDst());
}

test(AutoZoneSpecTest, createAbbreviation) {
  const uint8_t kDstSize = 6;
  char dst[kDstSize];

  AutoZoneSpec::createAbbreviation(dst, kDstSize, "SAST", 0, '\0');
  assertEqual("SAST", dst);

  AutoZoneSpec::createAbbreviation(dst, kDstSize, "P%T", 4, 'D');
  assertEqual("PDT", dst);

  AutoZoneSpec::createAbbreviation(dst, kDstSize, "P%T", 0, 'S');
  assertEqual("PST", dst);

  AutoZoneSpec::createAbbreviation(dst, kDstSize, "P%T", 0, '-');
  assertEqual("PT", dst);

  AutoZoneSpec::createAbbreviation(dst, kDstSize, "GMT/BST", 0, '-');
  assertEqual("GMT", dst);

  AutoZoneSpec::createAbbreviation(dst, kDstSize, "GMT/BST", 4, '-');
  assertEqual("BST", dst);

  AutoZoneSpec::createAbbreviation(dst, kDstSize, "P%T3456", 4, 'D');
  assertEqual("PDT34", dst);
}

// --------------------------------------------------------------------------
// Default TimeZone
// --------------------------------------------------------------------------

test(TimeZoneTest_Manual, default) {
  TimeZone tz;

  assertEqual(TimeZone::kTypeManual, tz.getType());
  assertEqual(0, tz.getUtcOffset(0).toMinutes());
  assertEqual("UTC", tz.getAbbrev(0));
  assertFalse(tz.getDst(0));
}

// --------------------------------------------------------------------------
// Manual TimeZone
// --------------------------------------------------------------------------

test(TimeZoneTest_Manual, operatorEqualEqual) {
  // PST
  ManualZoneSpec spa(
      UtcOffset::forHour(-8), UtcOffset::forHour(1), "PST", "PDT");
  ManualZoneSpec spb(
      UtcOffset::forHour(-8), UtcOffset::forHour(1), "PST", "PDT");

  // Two time zones with same zoneSpec should be equal.
  TimeZone a(&spa);
  TimeZone b(&spb);
  assertTrue(a == b);

  // One of them goes to DST. Should be different.
  spb.isDst(true);
  assertTrue(a != b);

  // Should be different from EST.
  ManualZoneSpec spc(
      UtcOffset::forHour(-5), UtcOffset::forHour(1), "EST", "EDT");
  TimeZone c(&spc);
  assertTrue(a != c);
}

test(TimeZoneTest_Manual, forUtcOffset) {
  ManualZoneSpec zoneSpec(
      UtcOffset::forHour(-8), UtcOffset::forHour(1), "PST", "PDT");
  TimeZone tz(&zoneSpec);

  assertEqual(TimeZone::kTypeManual, tz.getType());
  assertEqual(-8*60, tz.getUtcOffset(0).toMinutes());
  assertEqual("PST", tz.getAbbrev(0));
  assertFalse(tz.getDst(0));

  zoneSpec.isDst(true);
  assertEqual(-7*60, tz.getUtcOffset(0).toMinutes());
  assertEqual("PDT", tz.getAbbrev(0));
  assertTrue(tz.getDst(0));
}

// --------------------------------------------------------------------------
// Auto TimeZone
// --------------------------------------------------------------------------

test(TimeZoneTest_Auto, operatorEqualEqual) {
  AutoZoneSpec zoneSpecLA(&zonedb::kZoneLos_Angeles);
  AutoZoneSpec zoneSpecNY(&zonedb::kZoneNew_York);
  TimeZone a(&zoneSpecLA);
  TimeZone b(&zoneSpecNY);

  assertTrue(a != b);
}

test(TimeZoneTest_Auto, copyConstructor) {
  AutoZoneSpec zoneSpec(&zonedb::kZoneLos_Angeles);
  TimeZone a(&zoneSpec);
  TimeZone b(a);
  assertTrue(a == b);
}

test(TimeZoneTest_Auto, default) {
  TimeZone tz;
  assertEqual(TimeZone::kTypeManual, tz.getType());
  assertEqual(0, tz.getUtcOffset(0).toMinutes());
  assertEqual("UTC", tz.getAbbrev(0));
  assertFalse(tz.getDst(0));
}

test(TimeZoneTest_Auto, LosAngeles) {
  AutoZoneSpec zoneSpec(&zonedb::kZoneLos_Angeles);

  OffsetDateTime dt;
  acetime_t epochSeconds;

  TimeZone tz(&zoneSpec);
  assertEqual(TimeZone::kTypeAuto, tz.getType());

  dt = OffsetDateTime::forComponents(2018, 3, 11, 1, 59, 59,
      UtcOffset::forHour(-8));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-8*60, tz.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("PST", tz.getAbbrev(epochSeconds));
  assertFalse(tz.getDst(epochSeconds));

  dt = OffsetDateTime::forComponents(2018, 3, 11, 2, 0, 0,
      UtcOffset::forHour(-8));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-7*60, tz.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("PDT", tz.getAbbrev(epochSeconds));
  assertTrue(tz.getDst(epochSeconds));
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
