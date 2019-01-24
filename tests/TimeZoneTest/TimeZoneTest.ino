#line 2 "TimeZoneTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time;
using namespace ace_time::common;

// --------------------------------------------------------------------------
// DefaultZoneAgent
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
// AutoZoneAgent
// --------------------------------------------------------------------------

test(AutoZoneAgentTest, calcStartDayOfMonth) {
  // 2018-11, Sun>=1
  assertEqual(4, AutoZoneAgent::calcStartDayOfMonth(
      2018, 11, LocalDate::kSunday, 1));

  // 2018-11, lastSun
  assertEqual(25, AutoZoneAgent::calcStartDayOfMonth(
      2018, 11, LocalDate::kSunday, 0));

  // 2018-03, Thu>=9
  assertEqual(15, AutoZoneAgent::calcStartDayOfMonth(
      2018, 3, LocalDate::kThursday, 9));

  // 2018-03-30
  assertEqual(30, AutoZoneAgent::calcStartDayOfMonth(2018, 3, 0, 30));
}

test(AutoZoneAgentTest, calcRuleOffsetCode) {
  assertEqual(0, AutoZoneAgent::calcRuleOffsetCode(1, 2, 'u'));
  assertEqual(1, AutoZoneAgent::calcRuleOffsetCode(1, 2, 'w'));
  assertEqual(2, AutoZoneAgent::calcRuleOffsetCode(1, 2, 's'));
}

test(AutoZoneAgentTest, init_primitives) {
  AutoZoneAgent agent(&zonedb::kZoneLos_Angeles);
  agent.mYear = 2001;
  agent.mNumMatches = 0;

  agent.addRulePriorToYear(2001);
  assertEqual(0, agent.mNumMatches);
  assertEqual(-32, agent.mPreviousMatch.era->offsetCode);
  assertEqual("P%T", agent.mPreviousMatch.era->format);
  assertEqual(1967-2000, agent.mPreviousMatch.rule->fromYearTiny);
  assertEqual(2006-2000, agent.mPreviousMatch.rule->toYearTiny);
  assertEqual(10, agent.mPreviousMatch.rule->inMonth);

  agent.addRulesForYear(2001);
  assertEqual(2, agent.mNumMatches);

  assertEqual(-32, agent.mMatches[0].era->offsetCode);
  assertEqual("P%T", agent.mMatches[0].era->format);
  assertEqual(1987-2000, agent.mMatches[0].rule->fromYearTiny);
  assertEqual(2006-2000, agent.mMatches[0].rule->toYearTiny);
  assertEqual(4, agent.mMatches[0].rule->inMonth);

  assertEqual(-32, agent.mMatches[1].era->offsetCode);
  assertEqual("P%T", agent.mMatches[1].era->format);
  assertEqual(1967-2000, agent.mMatches[1].rule->fromYearTiny);
  assertEqual(2006-2000, agent.mMatches[1].rule->toYearTiny);
  assertEqual(10, agent.mMatches[1].rule->inMonth);

  agent.calcTransitions();
  assertEqual((acetime_t) 0, agent.mPreviousMatch.startEpochSeconds);
  assertEqual(-32, agent.mPreviousMatch.offsetCode);

  // t >= 2001-04-01 02:00 UTC-08:00 Sunday goes to PDT
  assertEqual(-28, agent.mMatches[0].offsetCode);
  assertEqual((acetime_t) 39434400, agent.mMatches[0].startEpochSeconds);

  // t >= 2001-10-28 02:00 UTC-07:00 Sunday goes to PST
  assertEqual(-32, agent.mMatches[1].offsetCode);
  assertEqual((acetime_t) 57574800, agent.mMatches[1].startEpochSeconds);
}

test(AutoZoneAgentTest, init) {
  AutoZoneAgent agent(&zonedb::kZoneLos_Angeles);
  LocalDate ld = LocalDate::forComponents(2018, 1, 1); // 2018-01-01
  agent.init(ld);

  assertEqual(2, agent.mNumMatches);

  assertEqual(-32, agent.mPreviousMatch.era->offsetCode);
  assertEqual("P%T", agent.mPreviousMatch.era->format);
  assertEqual(2007-2000, agent.mPreviousMatch.rule->fromYearTiny);
  assertEqual(ZoneRule::kMaxYearTiny,
      agent.mPreviousMatch.rule->toYearTiny);
  assertEqual(11, agent.mPreviousMatch.rule->inMonth);

  assertEqual(-32, agent.mMatches[0].era->offsetCode);
  assertEqual("P%T", agent.mMatches[0].era->format);
  assertEqual(2007-2000, agent.mMatches[0].rule->fromYearTiny);
  assertEqual(ZoneRule::kMaxYearTiny, agent.mMatches[0].rule->toYearTiny);
  assertEqual(3, agent.mMatches[0].rule->inMonth);

  assertEqual(-32, agent.mMatches[1].era->offsetCode);
  assertEqual("P%T", agent.mMatches[1].era->format);
  assertEqual(2007-2000, agent.mMatches[1].rule->fromYearTiny);
  assertEqual(ZoneRule::kMaxYearTiny, agent.mMatches[1].rule->toYearTiny);
  assertEqual(11, agent.mMatches[1].rule->inMonth);

  assertEqual((acetime_t) 0, agent.mPreviousMatch.startEpochSeconds);
  assertEqual(-32, agent.mPreviousMatch.offsetCode);

  // t >= 2018-03-11 02:00 UTC-08:00 Sunday goes to PDT
  assertEqual(-28, agent.mMatches[0].offsetCode);
  assertEqual((acetime_t) 574077600, agent.mMatches[0].startEpochSeconds);

  // t >= 2018-11-04 02:00 UTC-07:00 Sunday goes to PST
  assertEqual(-32, agent.mMatches[1].offsetCode);
  assertEqual((acetime_t) 594637200, agent.mMatches[1].startEpochSeconds);
}

// zoneInfo == nullptr means UTC
test(AutoZoneAgentTest, nullptr) {
  AutoZoneAgent agent(nullptr);
  assertEqual(0, agent.getUtcOffset(0).toMinutes());
  assertEqual("UTC", agent.getAbbrev(0));
  assertFalse(agent.getDeltaOffset(0).isDst());
}

test(AutoZoneAgentTest, copyConstructorAssignmentOperator) {
  OffsetDateTime dt = OffsetDateTime::forComponents(2018, 3, 11, 1, 59, 59,
      UtcOffset::forHour(-8));
  acetime_t epochSeconds = dt.toEpochSeconds();

  AutoZoneAgent m1(nullptr);
  assertEqual(0, m1.getUtcOffset(0).toMinutes());

  AutoZoneAgent m2(&zonedb::kZoneLos_Angeles);
  assertEqual(-8*60, m2.getUtcOffset(epochSeconds).toMinutes());

  m1 = m2;
  assertEqual(-8*60, m1.getUtcOffset(0).toMinutes());

  AutoZoneAgent m3(m2);
  assertEqual(-8*60, m1.getUtcOffset(0).toMinutes());
}

// https://www.timeanddate.com/time/zone/usa/los-angeles
test(AutoZoneAgentTest, kZoneLos_Angeles) {
  AutoZoneAgent agent(&zonedb::kZoneLos_Angeles);
  OffsetDateTime dt;
  acetime_t epochSeconds;

  dt = OffsetDateTime::forComponents(2018, 3, 11, 1, 59, 59,
      UtcOffset::forHour(-8));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-8*60, agent.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("PST", agent.getAbbrev(epochSeconds));
  assertFalse(agent.getDeltaOffset(epochSeconds).isDst());

  dt = OffsetDateTime::forComponents(2018, 3, 11, 2, 0, 0,
      UtcOffset::forHour(-8));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-7*60, agent.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("PDT", agent.getAbbrev(epochSeconds));
  assertTrue(agent.getDeltaOffset(epochSeconds).isDst());

  dt = OffsetDateTime::forComponents(2018, 11, 4, 1, 0, 0,
      UtcOffset::forHour(-7));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-7*60, agent.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("PDT", agent.getAbbrev(epochSeconds));
  assertTrue(agent.getDeltaOffset(epochSeconds).isDst());

  dt = OffsetDateTime::forComponents(2018, 11, 4, 1, 59, 59,
      UtcOffset::forHour(-7));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-7*60, agent.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("PDT", agent.getAbbrev(epochSeconds));
  assertTrue(agent.getDeltaOffset(epochSeconds).isDst());

  dt = OffsetDateTime::forComponents(2018, 11, 4, 2, 0, 0,
      UtcOffset::forHour(-7));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-8*60, agent.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("PST", agent.getAbbrev(epochSeconds));
  assertFalse(agent.getDeltaOffset(epochSeconds).isDst());
}

// https://www.timeanddate.com/time/zone/australia/sydney
test(AutoZoneAgentTest, kZoneSydney) {
  AutoZoneAgent agent(&zonedb::kZoneSydney);
  OffsetDateTime dt;
  acetime_t epochSeconds;

  dt = OffsetDateTime::forComponents(2007, 3, 25, 2, 59, 59,
      UtcOffset::forHour(11));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(11*60, agent.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("AEDT", agent.getAbbrev(epochSeconds));
  assertTrue(agent.getDeltaOffset(epochSeconds).isDst());

  dt = OffsetDateTime::forComponents(2007, 3, 25, 3, 0, 0,
      UtcOffset::forHour(11));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(10*60, agent.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("AEST", agent.getAbbrev(epochSeconds));
  assertFalse(agent.getDeltaOffset(epochSeconds).isDst());

  dt = OffsetDateTime::forComponents(2007, 10, 28, 1, 59, 59,
      UtcOffset::forHour(10));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(10*60, agent.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("AEST", agent.getAbbrev(epochSeconds));
  assertFalse(agent.getDeltaOffset(epochSeconds).isDst());

  dt = OffsetDateTime::forComponents(2007, 10, 28, 2, 0, 0,
      UtcOffset::forHour(10));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(11*60, agent.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("AEDT", agent.getAbbrev(epochSeconds));
  assertTrue(agent.getDeltaOffset(epochSeconds).isDst());
}

// https://www.timeanddate.com/time/zone/south-africa/johannesburg
// No DST changes at all.
test(AutoZoneAgentTest, kZoneJohannesburg) {
  AutoZoneAgent agent(&zonedb::kZoneJohannesburg);
  OffsetDateTime dt;
  acetime_t epochSeconds;

  dt = OffsetDateTime::forComponents(2018, 1, 1, 0, 0, 0,
      UtcOffset::forHour(2));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(2*60, agent.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("SAST", agent.getAbbrev(epochSeconds));
  assertFalse(agent.getDeltaOffset(epochSeconds).isDst());
}

// https://www.timeanddate.com/time/zone/australia/darwin
// No DST changes since 1944. Uses the last transition which occurred in March
// 1944.
test(AutoZoneAgentTest, kZoneDarwin) {
  AutoZoneAgent agent(&zonedb::kZoneDarwin);
  OffsetDateTime dt;
  acetime_t epochSeconds;

  dt = OffsetDateTime::forComponents(2018, 1, 1, 0, 0, 0,
      UtcOffset::forHourMinute(1, 9, 30));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(9*60+30, agent.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("ACST", agent.getAbbrev(epochSeconds));
  assertFalse(agent.getDeltaOffset(epochSeconds).isDst());
}

test(AutoZoneAgentTest, createAbbreviation) {
  const uint8_t kDstSize = 6;
  char dst[kDstSize];

  AutoZoneAgent::createAbbreviation(dst, kDstSize, "SAST", 0, '\0');
  assertEqual("SAST", dst);

  AutoZoneAgent::createAbbreviation(dst, kDstSize, "P%T", 4, 'D');
  assertEqual("PDT", dst);

  AutoZoneAgent::createAbbreviation(dst, kDstSize, "P%T", 0, 'S');
  assertEqual("PST", dst);

  AutoZoneAgent::createAbbreviation(dst, kDstSize, "P%T", 0, '-');
  assertEqual("PT", dst);

  AutoZoneAgent::createAbbreviation(dst, kDstSize, "GMT/BST", 0, '-');
  assertEqual("GMT", dst);

  AutoZoneAgent::createAbbreviation(dst, kDstSize, "GMT/BST", 4, '-');
  assertEqual("BST", dst);

  AutoZoneAgent::createAbbreviation(dst, kDstSize, "P%T3456", 4, 'D');
  assertEqual("PDT34", dst);
}

// --------------------------------------------------------------------------
// Default TimeZone
// --------------------------------------------------------------------------

test(TimeZoneTest, default) {
  TimeZone a;
  TimeZone b;

  assertTrue(a == b);
  assertFalse(a.isDst());
  assertEqual(TimeZone::kTypeDefault, a.getType());
}

// --------------------------------------------------------------------------
// Manual TimeZone
// --------------------------------------------------------------------------

test(TimeZoneTest_Manual, operatorEqualEqual) {
  ManualZoneAgent pstAgent(
      UtcOffset::forHour(-8), "PST", UtcOffset::forHour(1), "PDT");

  // Two time zones with same agent should be equal.
  TimeZone a(&pstAgent);
  TimeZone b(&pstAgent);
  assertTrue(a == b);

  // One of them goes to DST. Should be different.
  b.isDst(true);
  assertTrue(a != b);

  // Should be different from EST.
  ManualZoneAgent estAgent(
      UtcOffset::forHour(-5), "EST", UtcOffset::forHour(1), "EDT");
  TimeZone c(&estAgent);
  assertTrue(a != c);

  ManualZoneAgent pstAgent2(
      UtcOffset::forHour(-8), "PPP", UtcOffset::forHour(1), "QQQ");
  TimeZone d(&pstAgent2);
  assertTrue(a != d);
}

test(TimeZoneTest_Manual, copyConstructor) {
  ManualZoneAgent agent(
      UtcOffset::forHour(-8), "PST", UtcOffset::forHour(1), "PDT");
  TimeZone a(&agent);
  TimeZone b = a;
  assertTrue(a == b);
}

test(TimeZoneTest_Manual, default) {
  TimeZone tz;

  assertEqual(TimeZone::kTypeDefault, tz.getType());
  assertEqual(0, tz.getUtcOffset(0).toMinutes());
  assertEqual("UTC", tz.getAbbrev(0));
  assertFalse(tz.getDst(0));

  tz.isDst(true);
  assertEqual(0, tz.getUtcOffset(0).toMinutes());
  assertEqual("UTC", tz.getAbbrev(0));
  assertFalse(tz.getDst(0));
}

test(TimeZoneTest_Manual, forUtcOffset) {
  ManualZoneAgent agent(
      UtcOffset::forHour(-8), "PST", UtcOffset::forHour(1), "PDT");
  TimeZone tz(&agent);

  assertEqual(TimeZone::kTypeManual, tz.getType());
  assertEqual(-8*60, tz.getUtcOffset(0).toMinutes());
  assertEqual("PST", tz.getAbbrev(0));
  assertFalse(tz.getDst(0));

  tz.isDst(true);
  assertEqual(-7*60, tz.getUtcOffset(0).toMinutes());
  assertEqual("PDT", tz.getAbbrev(0));
  assertTrue(tz.getDst(0));
}

// --------------------------------------------------------------------------
// Auto TimeZone
// --------------------------------------------------------------------------

test(TimeZoneTest_Auto, operatorEqualEqual) {
  AutoZoneAgent agentLA(&zonedb::kZoneLos_Angeles);
  AutoZoneAgent agentNY(&zonedb::kZoneNew_York);
  TimeZone a(&agentLA);
  TimeZone b(&agentNY);

  assertTrue(a != b);
}

test(TimeZoneTest_Auto, copyConstructor) {
  AutoZoneAgent zoneAgent(&zonedb::kZoneLos_Angeles);
  TimeZone a(&zoneAgent);
  TimeZone b(a);
  assertTrue(a == b);
}

test(TimeZoneTest_Auto, default) {
  TimeZone tz;
  assertEqual(TimeZone::kTypeDefault, tz.getType());
  assertEqual(0, tz.getUtcOffset(0).toMinutes());
  assertEqual("UTC", tz.getAbbrev(0));
  assertFalse(tz.getDst(0));
}

test(TimeZoneTest_Auto, LosAngeles) {
  AutoZoneAgent agent(&zonedb::kZoneLos_Angeles);

  OffsetDateTime dt;
  acetime_t epochSeconds;

  TimeZone tz(&agent);
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
