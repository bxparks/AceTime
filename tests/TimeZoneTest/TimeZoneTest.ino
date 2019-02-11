#line 2 "TimeZoneTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time;
using namespace ace_time::zonedb;

// --------------------------------------------------------------------------
// ManualZoneSpecifier
// --------------------------------------------------------------------------

test(ManualZoneSpecifierTest, accessors) {
  ManualZoneSpecifier pstSpec(
      UtcOffset::forHour(-8), UtcOffset::forHour(1), "PST", "PDT");

  assertEqual(-8*60, pstSpec.getUtcOffset(0).toMinutes());
  assertEqual("PST", pstSpec.getAbbrev(0));
  assertEqual(0, pstSpec.getDeltaOffset(0).toMinutes());

  pstSpec.isDst(true);

  assertEqual(-7*60, pstSpec.getUtcOffset(0).toMinutes());
  assertEqual("PDT", pstSpec.getAbbrev(0));
  assertEqual(1*60, pstSpec.getDeltaOffset(0).toMinutes());
}

test(ManualZoneSpecifierTest, copyConstructor) {
  ManualZoneSpecifier a(
      UtcOffset::forHour(-8), UtcOffset::forHour(1), "PST", "PDT");
  ManualZoneSpecifier b(a);

  assertEqual(a.isDst(), b.isDst());
  assertEqual(a.stdOffset().toMinutes(), b.stdOffset().toMinutes());
  assertEqual(a.stdAbbrev(), b.stdAbbrev());
  assertEqual(a.deltaOffset().toMinutes(), b.deltaOffset().toMinutes());
  assertEqual(a.dstAbbrev(), b.dstAbbrev());

  b.isDst(true);
  assertNotEqual(a.isDst(), b.isDst());
}

test(ManualZoneSpecifierTest, operatorEqualEqual) {
  ManualZoneSpecifier a(
      UtcOffset::forHour(1), UtcOffset::forHour(1), "a", "b");
  ManualZoneSpecifier b(
      UtcOffset::forHour(2), UtcOffset::forHour(1), "a", "b");
  ManualZoneSpecifier c(
      UtcOffset::forHour(1), UtcOffset::forHour(1), "A", "b");
  ManualZoneSpecifier d(
      UtcOffset::forHour(1), UtcOffset::forHour(2), "a", "b");
  ManualZoneSpecifier e(
      UtcOffset::forHour(1), UtcOffset::forHour(1), "a", "B");

  assertTrue(a != b);
  assertTrue(a != c);
  assertTrue(a != d);
  assertTrue(a != e);

  ManualZoneSpecifier aa(a);
  assertTrue(a == aa);

  aa.isDst(true);
  assertTrue(a != aa);
}

// --------------------------------------------------------------------------
// AutoZoneSpecifier
// --------------------------------------------------------------------------

test(AutoZoneSpecifierTest, operatorEqualEqual) {
  AutoZoneSpecifier a(&zonedb::kZoneLos_Angeles);
  AutoZoneSpecifier b(&zonedb::kZoneDarwin);
  assertTrue(a != b);
}

test(AutoZoneSpecifierTest, calcStartDayOfMonth) {
  // 2018-11, Sun>=1
  assertEqual(4, AutoZoneSpecifier::calcStartDayOfMonth(
      2018, 11, LocalDate::kSunday, 1));

  // 2018-11, lastSun
  assertEqual(25, AutoZoneSpecifier::calcStartDayOfMonth(
      2018, 11, LocalDate::kSunday, 0));

  // 2018-03, Thu>=9
  assertEqual(15, AutoZoneSpecifier::calcStartDayOfMonth(
      2018, 3, LocalDate::kThursday, 9));

  // 2018-03-30
  assertEqual(30, AutoZoneSpecifier::calcStartDayOfMonth(2018, 3, 0, 30));
}

test(AutoZoneSpecifierTest, calcRuleOffsetCode) {
  assertEqual(0, AutoZoneSpecifier::calcRuleOffsetCode(1, 2, 'u'));
  assertEqual(1, AutoZoneSpecifier::calcRuleOffsetCode(1, 2, 'w'));
  assertEqual(2, AutoZoneSpecifier::calcRuleOffsetCode(1, 2, 's'));
}

test(AutoZoneSpecifierTest, init_primitives) {
  AutoZoneSpecifier zoneSpecifier(&zonedb::kZoneLos_Angeles);
  zoneSpecifier.mYear = 2001;
  zoneSpecifier.mNumTransitions = 0;

  zoneSpecifier.addRulePriorToYear(2001);
  assertEqual(0, zoneSpecifier.mNumTransitions);
  assertEqual(-32, zoneSpecifier.mPrevTransition.era->offsetCode);
  assertEqual("P%T", zoneSpecifier.mPrevTransition.era->format);
  assertEqual(1967-2000, zoneSpecifier.mPrevTransition.rule->fromYearTiny);
  assertEqual(2006-2000, zoneSpecifier.mPrevTransition.rule->toYearTiny);
  assertEqual(10, zoneSpecifier.mPrevTransition.rule->inMonth);

  zoneSpecifier.addRulesForYear(2001);
  assertEqual(2, zoneSpecifier.mNumTransitions);

  assertEqual(-32, zoneSpecifier.mTransitions[0].era->offsetCode);
  assertEqual("P%T", zoneSpecifier.mTransitions[0].era->format);
  assertEqual(1987-2000, zoneSpecifier.mTransitions[0].rule->fromYearTiny);
  assertEqual(2006-2000, zoneSpecifier.mTransitions[0].rule->toYearTiny);
  assertEqual(4, zoneSpecifier.mTransitions[0].rule->inMonth);

  assertEqual(-32, zoneSpecifier.mTransitions[1].era->offsetCode);
  assertEqual("P%T", zoneSpecifier.mTransitions[1].era->format);
  assertEqual(1967-2000, zoneSpecifier.mTransitions[1].rule->fromYearTiny);
  assertEqual(2006-2000, zoneSpecifier.mTransitions[1].rule->toYearTiny);
  assertEqual(10, zoneSpecifier.mTransitions[1].rule->inMonth);

  zoneSpecifier.calcTransitions();
  assertEqual((acetime_t) AutoZoneSpecifier::kMinEpochSeconds,
      zoneSpecifier.mPrevTransition.startEpochSeconds);
  assertEqual(-32, zoneSpecifier.mPrevTransition.offsetCode);

  // t >= 2001-04-01 02:00 UTC-08:00 Sunday goes to PDT
  assertEqual(-28, zoneSpecifier.mTransitions[0].offsetCode);
  assertEqual((acetime_t) 39434400,
      zoneSpecifier.mTransitions[0].startEpochSeconds);

  // t >= 2001-10-28 02:00 UTC-07:00 Sunday goes to PST
  assertEqual(-32, zoneSpecifier.mTransitions[1].offsetCode);
  assertEqual((acetime_t) 57574800,
      zoneSpecifier.mTransitions[1].startEpochSeconds);
}

test(AutoZoneSpecifierTest, init) {
  // Test using 2018-01-02. If we use 2018-01-01, the code will populate the
  // cache with transitions from 2017.
  AutoZoneSpecifier zoneSpecifier(&zonedb::kZoneLos_Angeles);
  LocalDate ld = LocalDate::forComponents(2018, 1, 2);
  zoneSpecifier.init(ld);

  assertEqual(2, zoneSpecifier.mNumTransitions);

  assertEqual(-32, zoneSpecifier.mPrevTransition.era->offsetCode);
  assertEqual("P%T", zoneSpecifier.mPrevTransition.era->format);
  assertEqual(2007-2000, zoneSpecifier.mPrevTransition.rule->fromYearTiny);
  assertEqual(common::ZoneRule::kMaxYearTiny,
      zoneSpecifier.mPrevTransition.rule->toYearTiny);
  assertEqual(11, zoneSpecifier.mPrevTransition.rule->inMonth);

  assertEqual(-32, zoneSpecifier.mTransitions[0].era->offsetCode);
  assertEqual("P%T", zoneSpecifier.mTransitions[0].era->format);
  assertEqual(2007-2000, zoneSpecifier.mTransitions[0].rule->fromYearTiny);
  assertEqual(common::ZoneRule::kMaxYearTiny,
      zoneSpecifier.mTransitions[0].rule->toYearTiny);
  assertEqual(3, zoneSpecifier.mTransitions[0].rule->inMonth);

  assertEqual(-32, zoneSpecifier.mTransitions[1].era->offsetCode);
  assertEqual("P%T", zoneSpecifier.mTransitions[1].era->format);
  assertEqual(2007-2000, zoneSpecifier.mTransitions[1].rule->fromYearTiny);
  assertEqual(common::ZoneRule::kMaxYearTiny,
      zoneSpecifier.mTransitions[1].rule->toYearTiny);
  assertEqual(11, zoneSpecifier.mTransitions[1].rule->inMonth);

  assertEqual((acetime_t) AutoZoneSpecifier::kMinEpochSeconds,
      zoneSpecifier.mPrevTransition.startEpochSeconds);
  assertEqual(-32, zoneSpecifier.mPrevTransition.offsetCode);

  // t >= 2018-03-11 02:00 UTC-08:00 Sunday goes to PDT
  assertEqual(-28, zoneSpecifier.mTransitions[0].offsetCode);
  assertEqual((acetime_t) 574077600,
      zoneSpecifier.mTransitions[0].startEpochSeconds);

  // t >= 2018-11-04 02:00 UTC-07:00 Sunday goes to PST
  assertEqual(-32, zoneSpecifier.mTransitions[1].offsetCode);
  assertEqual((acetime_t) 594637200,
      zoneSpecifier.mTransitions[1].startEpochSeconds);
}

// zoneInfo == nullptr means UTC
test(AutoZoneSpecifierTest, nullptr) {
  AutoZoneSpecifier zoneSpecifier(nullptr);
  assertEqual(0, zoneSpecifier.getUtcOffset(0).toMinutes());
  assertEqual("UTC", zoneSpecifier.getAbbrev(0));
  assertFalse(zoneSpecifier.getDeltaOffset(0).isDst());
}

test(AutoZoneSpecifierTest, copyConstructorAssignmentOperator) {
  OffsetDateTime dt = OffsetDateTime::forComponents(2018, 3, 11, 1, 59, 59,
      UtcOffset::forHour(-8));
  acetime_t epochSeconds = dt.toEpochSeconds();

  AutoZoneSpecifier m1(nullptr);
  assertEqual(0, m1.getUtcOffset(0).toMinutes());

  AutoZoneSpecifier m2(&zonedb::kZoneLos_Angeles);
  assertEqual(-8*60, m2.getUtcOffset(epochSeconds).toMinutes());

  m1 = m2;
  assertEqual(-8*60, m1.getUtcOffset(0).toMinutes());

  AutoZoneSpecifier m3(m2);
  assertEqual(-8*60, m1.getUtcOffset(0).toMinutes());
}

// https://www.timeanddate.com/time/zone/usa/los-angeles
test(AutoZoneSpecifierTest, kZoneLos_Angeles) {
  AutoZoneSpecifier zoneSpecifier(&zonedb::kZoneLos_Angeles);
  OffsetDateTime dt;
  acetime_t epochSeconds;

  dt = OffsetDateTime::forComponents(2018, 3, 11, 1, 59, 59,
      UtcOffset::forHour(-8));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-8*60, zoneSpecifier.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("PST", zoneSpecifier.getAbbrev(epochSeconds));
  assertFalse(zoneSpecifier.getDeltaOffset(epochSeconds).isDst());

  dt = OffsetDateTime::forComponents(2018, 3, 11, 2, 0, 0,
      UtcOffset::forHour(-8));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-7*60, zoneSpecifier.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("PDT", zoneSpecifier.getAbbrev(epochSeconds));
  assertTrue(zoneSpecifier.getDeltaOffset(epochSeconds).isDst());

  dt = OffsetDateTime::forComponents(2018, 11, 4, 1, 0, 0,
      UtcOffset::forHour(-7));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-7*60, zoneSpecifier.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("PDT", zoneSpecifier.getAbbrev(epochSeconds));
  assertTrue(zoneSpecifier.getDeltaOffset(epochSeconds).isDst());

  dt = OffsetDateTime::forComponents(2018, 11, 4, 1, 59, 59,
      UtcOffset::forHour(-7));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-7*60, zoneSpecifier.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("PDT", zoneSpecifier.getAbbrev(epochSeconds));
  assertTrue(zoneSpecifier.getDeltaOffset(epochSeconds).isDst());

  dt = OffsetDateTime::forComponents(2018, 11, 4, 2, 0, 0,
      UtcOffset::forHour(-7));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-8*60, zoneSpecifier.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("PST", zoneSpecifier.getAbbrev(epochSeconds));
  assertFalse(zoneSpecifier.getDeltaOffset(epochSeconds).isDst());
}

// https://www.timeanddate.com/time/zone/south-africa/johannesburg
// No DST changes at all.
test(AutoZoneSpecifierTest, kZoneJohannesburg) {
  AutoZoneSpecifier zoneSpecifier(&zonedb::kZoneJohannesburg);
  OffsetDateTime dt;
  acetime_t epochSeconds;

  dt = OffsetDateTime::forComponents(2018, 1, 1, 0, 0, 0,
      UtcOffset::forHour(2));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(2*60, zoneSpecifier.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("SAST", zoneSpecifier.getAbbrev(epochSeconds));
  assertFalse(zoneSpecifier.getDeltaOffset(epochSeconds).isDst());
}

// https://www.timeanddate.com/time/zone/australia/darwin
// No DST changes since 1944. Uses the last transition which occurred in March
// 1944.
test(AutoZoneSpecifierTest, kZoneDarwin) {
  AutoZoneSpecifier zoneSpecifier(&zonedb::kZoneDarwin);
  OffsetDateTime dt;
  acetime_t epochSeconds;

  dt = OffsetDateTime::forComponents(2018, 1, 1, 0, 0, 0,
      UtcOffset::forHourMinute(1, 9, 30));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(9*60+30, zoneSpecifier.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("ACST", zoneSpecifier.getAbbrev(epochSeconds));
  assertFalse(zoneSpecifier.getDeltaOffset(epochSeconds).isDst());
}

test(AutoZoneSpecifierTest, createAbbreviation) {
  const uint8_t kDstSize = 6;
  char dst[kDstSize];

  AutoZoneSpecifier::createAbbreviation(dst, kDstSize, "SAST", 0, '\0');
  assertEqual("SAST", dst);

  AutoZoneSpecifier::createAbbreviation(dst, kDstSize, "P%T", 4, 'D');
  assertEqual("PDT", dst);

  AutoZoneSpecifier::createAbbreviation(dst, kDstSize, "P%T", 0, 'S');
  assertEqual("PST", dst);

  AutoZoneSpecifier::createAbbreviation(dst, kDstSize, "P%T", 0, '-');
  assertEqual("PT", dst);

  AutoZoneSpecifier::createAbbreviation(dst, kDstSize, "GMT/BST", 0, '-');
  assertEqual("GMT", dst);

  AutoZoneSpecifier::createAbbreviation(dst, kDstSize, "GMT/BST", 4, '-');
  assertEqual("BST", dst);

  AutoZoneSpecifier::createAbbreviation(dst, kDstSize, "P%T3456", 4, 'D');
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
}

// --------------------------------------------------------------------------
// Manual TimeZone
// --------------------------------------------------------------------------

test(TimeZoneTest_Manual, operatorEqualEqual) {
  // PST
  ManualZoneSpecifier spa(
      UtcOffset::forHour(-8), UtcOffset::forHour(1), "PST", "PDT");
  ManualZoneSpecifier spb(
      UtcOffset::forHour(-8), UtcOffset::forHour(1), "PST", "PDT");

  // Two time zones with same zoneSpecifier should be equal.
  TimeZone a(&spa);
  TimeZone b(&spb);
  assertTrue(a == b);

  // One of them goes to DST. Should be different.
  spb.isDst(true);
  assertTrue(a != b);

  // Should be different from EST.
  ManualZoneSpecifier spc(
      UtcOffset::forHour(-5), UtcOffset::forHour(1), "EST", "EDT");
  TimeZone c(&spc);
  assertTrue(a != c);
}

test(TimeZoneTest_Manual, forUtcOffset) {
  ManualZoneSpecifier zoneSpecifier(
      UtcOffset::forHour(-8), UtcOffset::forHour(1), "PST", "PDT");
  TimeZone tz(&zoneSpecifier);

  assertEqual(TimeZone::kTypeManual, tz.getType());
  assertEqual(-8*60, tz.getUtcOffset(0).toMinutes());
  assertEqual("PST", tz.getAbbrev(0));

  zoneSpecifier.isDst(true);
  assertEqual(-7*60, tz.getUtcOffset(0).toMinutes());
  assertEqual("PDT", tz.getAbbrev(0));
}

// --------------------------------------------------------------------------
// Auto TimeZone
// --------------------------------------------------------------------------

test(TimeZoneTest_Auto, operatorEqualEqual) {
  AutoZoneSpecifier zoneSpecifierLA(&zonedb::kZoneLos_Angeles);
  AutoZoneSpecifier zoneSpecifierNY(&zonedb::kZoneNew_York);
  TimeZone a(&zoneSpecifierLA);
  TimeZone b(&zoneSpecifierNY);

  assertTrue(a != b);
}

test(TimeZoneTest_Auto, copyConstructor) {
  AutoZoneSpecifier zoneSpecifier(&zonedb::kZoneLos_Angeles);
  TimeZone a(&zoneSpecifier);
  TimeZone b(a);
  assertTrue(a == b);
}

test(TimeZoneTest_Auto, default) {
  TimeZone tz;
  assertEqual(TimeZone::kTypeManual, tz.getType());
  assertEqual(0, tz.getUtcOffset(0).toMinutes());
  assertEqual("UTC", tz.getAbbrev(0));
}

test(TimeZoneTest_Auto, LosAngeles) {
  AutoZoneSpecifier zoneSpecifier(&zonedb::kZoneLos_Angeles);

  OffsetDateTime dt;
  acetime_t epochSeconds;

  TimeZone tz(&zoneSpecifier);
  assertEqual(TimeZone::kTypeAuto, tz.getType());

  dt = OffsetDateTime::forComponents(2018, 3, 11, 1, 59, 59,
      UtcOffset::forHour(-8));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-8*60, tz.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("PST", tz.getAbbrev(epochSeconds));

  dt = OffsetDateTime::forComponents(2018, 3, 11, 2, 0, 0,
      UtcOffset::forHour(-8));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-7*60, tz.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("PDT", tz.getAbbrev(epochSeconds));
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
