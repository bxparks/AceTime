#line 2 "TimeZoneTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time;
using namespace ace_time::common;

// --------------------------------------------------------------------------
// ZoneAgent
// --------------------------------------------------------------------------

test(ZoneAgentTest, calcStartDayOfMonth) {
  // 2018-11, Sun>=1
  assertEqual(4, ZoneAgent::calcStartDayOfMonth(
      2018, 11, LocalDate::kSunday, 1));

  // 2018-11, lastSun
  assertEqual(25, ZoneAgent::calcStartDayOfMonth(
      2018, 11, LocalDate::kSunday, 0));

  // 2018-03, Thu>=9
  assertEqual(15, ZoneAgent::calcStartDayOfMonth(
      2018, 3, LocalDate::kThursday, 9));

  // 2018-03-30
  assertEqual(30, ZoneAgent::calcStartDayOfMonth(2018, 3, 0, 30));
}

test(ZoneAgentTest, calcRuleOffsetCode) {
  assertEqual(0, ZoneAgent::calcRuleOffsetCode(1, 2, 'u'));
  assertEqual(1, ZoneAgent::calcRuleOffsetCode(1, 2, 'w'));
  assertEqual(2, ZoneAgent::calcRuleOffsetCode(1, 2, 's'));
}

test(ZoneAgentTest, init_primitives) {
  ZoneAgent manager(&zonedb::kZoneLos_Angeles);
  manager.mYear = 2001;
  manager.mNumMatches = 0;

  manager.addRulePriorToYear(2001);
  assertEqual(0, manager.mNumMatches);
  assertEqual(-32, manager.mPreviousMatch.era->offsetCode);
  assertEqual("P%T", manager.mPreviousMatch.era->format);
  assertEqual((uint16_t) 1967, manager.mPreviousMatch.rule->fromYear);
  assertEqual((uint16_t) 2006, manager.mPreviousMatch.rule->toYear);
  assertEqual(10, manager.mPreviousMatch.rule->inMonth);

  manager.addRulesForYear(2001);
  assertEqual(2, manager.mNumMatches);

  assertEqual(-32, manager.mMatches[0].era->offsetCode);
  assertEqual("P%T", manager.mMatches[0].era->format);
  assertEqual((uint16_t) 1987, manager.mMatches[0].rule->fromYear);
  assertEqual((uint16_t) 2006, manager.mMatches[0].rule->toYear);
  assertEqual(4, manager.mMatches[0].rule->inMonth);

  assertEqual(-32, manager.mMatches[1].era->offsetCode);
  assertEqual("P%T", manager.mMatches[1].era->format);
  assertEqual((uint16_t) 1967, manager.mMatches[1].rule->fromYear);
  assertEqual((uint16_t) 2006, manager.mMatches[1].rule->toYear);
  assertEqual(10, manager.mMatches[1].rule->inMonth);

  manager.calcTransitions();
  assertEqual((uint32_t) 0, manager.mPreviousMatch.startEpochSeconds);
  assertEqual(-32, manager.mPreviousMatch.offsetCode);

  // t >= 2001-04-01 02:00 UTC-08:00 Sunday goes to PDT
  assertEqual(-28, manager.mMatches[0].offsetCode);
  assertEqual((uint32_t) 39434400, manager.mMatches[0].startEpochSeconds);

  // t >= 2001-10-28 02:00 UTC-07:00 Sunday goes to PST
  assertEqual(-32, manager.mMatches[1].offsetCode);
  assertEqual((uint32_t) 57574800, manager.mMatches[1].startEpochSeconds);
}

test(ZoneAgentTest, init) {
  ZoneAgent manager(&zonedb::kZoneLos_Angeles);
  LocalDate ld = LocalDate::forComponents(2018, 1, 1); // 2018-01-01
  manager.init(ld);

  assertEqual(2, manager.mNumMatches);

  assertEqual(-32, manager.mPreviousMatch.era->offsetCode);
  assertEqual("P%T", manager.mPreviousMatch.era->format);
  assertEqual((uint16_t) 2007, manager.mPreviousMatch.rule->fromYear);
  assertEqual(ZoneRule::kMaxYear, manager.mPreviousMatch.rule->toYear);
  assertEqual(11, manager.mPreviousMatch.rule->inMonth);

  assertEqual(-32, manager.mMatches[0].era->offsetCode);
  assertEqual("P%T", manager.mMatches[0].era->format);
  assertEqual((uint16_t) 2007, manager.mMatches[0].rule->fromYear);
  assertEqual(ZoneRule::kMaxYear, manager.mMatches[0].rule->toYear);
  assertEqual(3, manager.mMatches[0].rule->inMonth);

  assertEqual(-32, manager.mMatches[1].era->offsetCode);
  assertEqual("P%T", manager.mMatches[1].era->format);
  assertEqual((uint16_t) 2007, manager.mMatches[1].rule->fromYear);
  assertEqual(ZoneRule::kMaxYear, manager.mMatches[1].rule->toYear);
  assertEqual(11, manager.mMatches[1].rule->inMonth);

  assertEqual((uint32_t) 0, manager.mPreviousMatch.startEpochSeconds);
  assertEqual(-32, manager.mPreviousMatch.offsetCode);

  // t >= 2018-03-11 02:00 UTC-08:00 Sunday goes to PDT
  assertEqual(-28, manager.mMatches[0].offsetCode);
  assertEqual((uint32_t) 574077600, manager.mMatches[0].startEpochSeconds);

  // t >= 2018-11-04 02:00 UTC-07:00 Sunday goes to PST
  assertEqual(-32, manager.mMatches[1].offsetCode);
  assertEqual((uint32_t) 594637200, manager.mMatches[1].startEpochSeconds);
}

// zoneInfo == nullptr means UTC
test(ZoneAgentTest, nullptr) {
  ZoneAgent manager(nullptr);
  assertEqual(0, manager.getUtcOffset(0).toOffsetCode());
  assertEqual("UTC", manager.getAbbrev(0));
  assertFalse(manager.isDst(0));
}

test(ZoneAgentTest, copyConstructorAssignmentOperator) {
  OffsetDateTime dt = OffsetDateTime::forComponents(2018, 3, 11, 1, 59, 59,
      UtcOffset::forHour(-8));
  uint32_t epochSeconds = dt.toEpochSeconds();

  ZoneAgent m1(nullptr);
  assertEqual(0, m1.getUtcOffset(0).toOffsetCode());

  ZoneAgent m2(&zonedb::kZoneLos_Angeles);
  assertEqual(-32, m2.getUtcOffset(epochSeconds).toOffsetCode());

  m1 = m2;
  assertEqual(-32, m1.getUtcOffset(0).toOffsetCode());

  ZoneAgent m3(m2);
  assertEqual(-32, m1.getUtcOffset(0).toOffsetCode());
}

// https://www.timeanddate.com/time/zone/usa/los-angeles
test(ZoneAgentTest, kZoneLos_Angeles) {
  ZoneAgent manager(&zonedb::kZoneLos_Angeles);
  OffsetDateTime dt;
  uint32_t epochSeconds;

  dt = OffsetDateTime::forComponents(2018, 3, 11, 1, 59, 59,
      UtcOffset::forHour(-8));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-32, manager.getUtcOffset(epochSeconds).toOffsetCode());
  assertEqual("PST", manager.getAbbrev(epochSeconds));
  assertFalse(manager.isDst(epochSeconds));

  dt = OffsetDateTime::forComponents(2018, 3, 11, 2, 0, 0,
      UtcOffset::forHour(-8));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-28, manager.getUtcOffset(epochSeconds).toOffsetCode());
  assertEqual("PDT", manager.getAbbrev(epochSeconds));
  assertTrue(manager.isDst(epochSeconds));

  dt = OffsetDateTime::forComponents(2018, 11, 4, 1, 0, 0,
      UtcOffset::forHour(-7));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-28, manager.getUtcOffset(epochSeconds).toOffsetCode());
  assertEqual("PDT", manager.getAbbrev(epochSeconds));
  assertTrue(manager.isDst(epochSeconds));

  dt = OffsetDateTime::forComponents(2018, 11, 4, 1, 59, 59,
      UtcOffset::forHour(-7));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-28, manager.getUtcOffset(epochSeconds).toOffsetCode());
  assertEqual("PDT", manager.getAbbrev(epochSeconds));
  assertTrue(manager.isDst(epochSeconds));

  dt = OffsetDateTime::forComponents(2018, 11, 4, 2, 0, 0,
      UtcOffset::forHour(-7));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-32, manager.getUtcOffset(epochSeconds).toOffsetCode());
  assertEqual("PST", manager.getAbbrev(epochSeconds));
  assertFalse(manager.isDst(epochSeconds));
}

// https://www.timeanddate.com/time/zone/australia/sydney
test(ZoneAgentTest, kZoneSydney) {
  ZoneAgent manager(&zonedb::kZoneSydney);
  OffsetDateTime dt;
  uint32_t epochSeconds;

  dt = OffsetDateTime::forComponents(2007, 3, 25, 2, 59, 59,
      UtcOffset::forHour(11));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(44, manager.getUtcOffset(epochSeconds).toOffsetCode());
  assertEqual("AEDT", manager.getAbbrev(epochSeconds));
  assertTrue(manager.isDst(epochSeconds));

  dt = OffsetDateTime::forComponents(2007, 3, 25, 3, 0, 0,
      UtcOffset::forHour(11));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(40, manager.getUtcOffset(epochSeconds).toOffsetCode());
  assertEqual("AEST", manager.getAbbrev(epochSeconds));
  assertFalse(manager.isDst(epochSeconds));

  dt = OffsetDateTime::forComponents(2007, 10, 28, 1, 59, 59,
      UtcOffset::forHour(10));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(40, manager.getUtcOffset(epochSeconds).toOffsetCode());
  assertEqual("AEST", manager.getAbbrev(epochSeconds));
  assertFalse(manager.isDst(epochSeconds));

  dt = OffsetDateTime::forComponents(2007, 10, 28, 2, 0, 0,
      UtcOffset::forHour(10));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(44, manager.getUtcOffset(epochSeconds).toOffsetCode());
  assertEqual("AEDT", manager.getAbbrev(epochSeconds));
  assertTrue(manager.isDst(epochSeconds));
}

// https://www.timeanddate.com/time/zone/south-africa/johannesburg
// No DST changes at all.
test(ZoneAgentTest, kZoneJohannesburg) {
  ZoneAgent manager(&zonedb::kZoneJohannesburg);
  OffsetDateTime dt;
  uint32_t epochSeconds;

  dt = OffsetDateTime::forComponents(2018, 1, 1, 0, 0, 0,
      UtcOffset::forHour(2));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(8, manager.getUtcOffset(epochSeconds).toOffsetCode());
  assertEqual("SAST", manager.getAbbrev(epochSeconds));
  assertFalse(manager.isDst(epochSeconds));
}

// https://www.timeanddate.com/time/zone/australia/darwin
// No DST changes since 1944. Uses the last transition which occurred in March
// 1944.
test(ZoneAgentTest, kZoneDarwin) {
  ZoneAgent manager(&zonedb::kZoneDarwin);
  OffsetDateTime dt;
  uint32_t epochSeconds;

  dt = OffsetDateTime::forComponents(2018, 1, 1, 0, 0, 0,
      UtcOffset::forHourMinute(1, 9, 30));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(38, manager.getUtcOffset(epochSeconds).toOffsetCode());
  assertEqual("ACST", manager.getAbbrev(epochSeconds));
  assertFalse(manager.isDst(epochSeconds));
}

test(ZoneAgentTest, createAbbreviation) {
  const uint8_t kDstSize = 6;
  char dst[kDstSize];

  ZoneAgent::createAbbreviation(dst, kDstSize, "SAST", 0, '\0');
  assertEqual("SAST", dst);

  ZoneAgent::createAbbreviation(dst, kDstSize, "P%T", 4, 'D');
  assertEqual("PDT", dst);

  ZoneAgent::createAbbreviation(dst, kDstSize, "P%T", 0, 'S');
  assertEqual("PST", dst);

  ZoneAgent::createAbbreviation(dst, kDstSize, "P%T", 0, '-');
  assertEqual("PT", dst);

  ZoneAgent::createAbbreviation(dst, kDstSize, "GMT/BST", 0, '-');
  assertEqual("GMT", dst);

  ZoneAgent::createAbbreviation(dst, kDstSize, "GMT/BST", 4, '-');
  assertEqual("BST", dst);

  ZoneAgent::createAbbreviation(dst, kDstSize, "P%T3456", 4, 'D');
  assertEqual("PDT34", dst);
}

// --------------------------------------------------------------------------
// TimeZone
// --------------------------------------------------------------------------

test(TimeZone, assignmentOperator) {
  ZoneAgent agent(&zonedb::kZoneLos_Angeles);
  TimeZone a = TimeZone::forUtcOffset(
      UtcOffset::forHour(-8), false, "PST", "PDT");

  TimeZone b;
  b = a;
  assertTrue(a == b);
}

// --------------------------------------------------------------------------
// Manual TimeZone
// --------------------------------------------------------------------------

test(TimeZone_Manual, operatorEqualEqual) {
  TimeZone a = TimeZone::forUtcOffset(
      UtcOffset::forHour(-8), false, "PST", "PDT");
  TimeZone b = TimeZone::forUtcOffset(
      UtcOffset::forHour(-8), false, "PST", "PDT");
  assertTrue(a == b);

  b = TimeZone::forUtcOffset(UtcOffset::forHour(-7), false, "PST", "PDT");
  assertTrue(a != b);

  b = TimeZone::forUtcOffset(UtcOffset::forHour(-8), true, "PST", "PDT");
  assertTrue(a != b);

  b = TimeZone::forUtcOffset(UtcOffset::forHour(-8), false, "xxx", "PDT");
  assertTrue(a != b);

  b = TimeZone::forUtcOffset(UtcOffset::forHour(-8), false, "PST", "xxx");
  assertTrue(a != b);
}

test(TimeZone_Manual, copyConstructor) {
  TimeZone a = TimeZone::forUtcOffset(
      UtcOffset::forHour(-8), false, "PST", "PDT");
  TimeZone b(a);
  assertTrue(a == b);
}

test(TimeZone_Manual, default) {
  const TimeZone tz;
  assertEqual(TimeZone::kTypeManual, tz.getType());
  assertEqual(0, tz.utcOffset().toOffsetCode());
  assertEqual("UTC", tz.stdAbbrev());
  assertEqual("UTC", tz.dstAbbrev());

  assertEqual(0, tz.getUtcOffset(0).toOffsetCode());
  assertEqual("UTC", tz.getAbbrev(0));
  assertFalse(tz.getDst(0));
}

test(TimeZone_Manual, forUtcOffset) {
  TimeZone tz = TimeZone::forUtcOffset(
      UtcOffset::forHour(-8), false, "PST", "PDT");
  assertEqual(TimeZone::kTypeManual, tz.getType());
  assertEqual(-32, tz.utcOffset().toOffsetCode());
  assertEqual("PST", (uintptr_t) tz.stdAbbrev());
  assertEqual("PDT", (uintptr_t) tz.dstAbbrev());
  assertFalse(tz.isDst());

  assertEqual(-32, tz.getUtcOffset(0).toOffsetCode());
  assertEqual("PST", tz.getAbbrev(0));
  assertFalse(tz.getDst(0));

  tz.isDst(true);
  assertEqual(-28, tz.getUtcOffset(0).toOffsetCode());
  assertEqual("PDT", tz.getAbbrev(0));
  assertTrue(tz.getDst(0));
}

// --------------------------------------------------------------------------
// Auto TimeZone
// --------------------------------------------------------------------------

test(TimeZone_Auto, operatorEqualEqual) {
  ZoneAgent agentLA(&zonedb::kZoneLos_Angeles);
  ZoneAgent agentNY(&zonedb::kZoneNew_York);
  TimeZone a = TimeZone::forZone(&agentLA);
  TimeZone b = TimeZone::forZone(&agentNY);

  assertTrue(a != b);
}

test(TimeZone_Auto, copyConstructor) {
  ZoneAgent zoneAgent(&zonedb::kZoneLos_Angeles);
  TimeZone a = TimeZone::forZone(&zoneAgent);
  TimeZone b(a);
  assertTrue(a == b);
}

test(TimeZone_Auto, nullptr) {
  TimeZone tz = TimeZone::forZone(nullptr);
  assertEqual(TimeZone::kTypeAuto, tz.getType());
  assertEqual(0, tz.getUtcOffset(0).toOffsetCode());
  assertEqual("", tz.getAbbrev(0));
  assertFalse(tz.getDst(0));
}

test(TimeZone_Auto, LosAngeles) {
  ZoneAgent agent(&zonedb::kZoneLos_Angeles);

  OffsetDateTime dt;
  uint32_t epochSeconds;

  TimeZone tz = TimeZone::forZone(&agent);
  assertEqual(TimeZone::kTypeAuto, tz.getType());

  dt = OffsetDateTime::forComponents(2018, 3, 11, 1, 59, 59,
      UtcOffset::forHour(-8));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-32, tz.getUtcOffset(epochSeconds).toOffsetCode());
  assertEqual("PST", tz.getAbbrev(epochSeconds));
  assertFalse(tz.getDst(epochSeconds));

  dt = OffsetDateTime::forComponents(2018, 3, 11, 2, 0, 0,
      UtcOffset::forHour(-8));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-28, tz.getUtcOffset(epochSeconds).toOffsetCode());
  assertEqual("PDT", tz.getAbbrev(epochSeconds));
  assertTrue(tz.getDst(epochSeconds));
}

// --------------------------------------------------------------------------

void setup() {
  delay(1000); // wait for stability on some boards to prevent garbage Serial
  Serial.begin(115200); // ESP8266 default of 74880 not supported on Linux
  while(!Serial); // for the Arduino Leonardo/Micro only
}

void loop() {
  TestRunner::run();
}
