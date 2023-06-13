#line 2 "BasicZoneProcessorTest.ino"

#include <AUnit.h>
#include <AceCommon.h> // PrintStr<>
#include <AceTime.h>
#include <ace_time/testing/EpochYearContext.h>
#include <zonedbtesting/zone_policies.h>
#include <zonedbtesting/zone_infos.h>

using ace_common::PrintStr;
using namespace ace_time;
using ace_time::basic::compareYearMonth;
using ace_time::basic::ZoneContext;
using ace_time::basic::ZoneRule;
using ace_time::basic::ZonePolicy;
using ace_time::basic::ZoneEra;
using ace_time::basic::ZoneInfo;
using ace_time::basic::ZoneContextBroker;
using ace_time::basic::ZoneRuleBroker;
using ace_time::basic::ZonePolicyBroker;
using ace_time::basic::ZoneEraBroker;
using ace_time::basic::ZoneInfoBroker;
using ace_time::testing::EpochYearContext;
using ace_time::zonedbtesting::kZoneContext;
using ace_time::zonedbtesting::kZoneAmerica_Los_Angeles;
using ace_time::zonedbtesting::kZoneAustralia_Darwin;
using ace_time::zonedbtesting::kZonePacific_Galapagos;
using ace_time::zonedbtesting::kZonePolicyEcuador;
using ace_time::zonedbtesting::kZoneAfrica_Johannesburg;

//---------------------------------------------------------------------------
// BasicZoneProcessor: test private methods
//---------------------------------------------------------------------------

test(BasicZoneProcessorTest, compareYearMonth) {
  assertEqual(compareYearMonth(2000, 2, 2000, 1), 1);
  assertEqual(compareYearMonth(2000, 2, 2000, 2), 0);
  assertEqual(compareYearMonth(2000, 2, 2000, 3), -1);

  // Make sure that compareYearMonth() uses 'int16_t year' not 'int8_t year'
  assertEqual(compareYearMonth(127, 2, 128, 2), -1);
  assertEqual(compareYearMonth(128, 2, 128, 2), 0);
  assertEqual(compareYearMonth(256, 2, 128, 2), 1);
}

test(BasicZoneProcessorTest, operatorEqualEqual) {
  BasicZoneProcessor a(&kZoneAmerica_Los_Angeles);
  BasicZoneProcessor b(&kZoneAustralia_Darwin);
  assertTrue(a != b);
}

test(BasicZoneProcessorTest, calcRuleOffsetMinutes) {
  assertEqual(0, BasicZoneProcessor::calcRuleOffsetMinutes(1, 2,
      ZoneContext::kSuffixU));
  assertEqual(1, BasicZoneProcessor::calcRuleOffsetMinutes(1, 2,
      ZoneContext::kSuffixW));
  assertEqual(2, BasicZoneProcessor::calcRuleOffsetMinutes(1, 2,
      ZoneContext::kSuffixS));
}

// Pacific/Galapagos transitions from simple Rule to named Rule in 1986:
test(BasicZoneProcessorTest, findZoneEra) {
  ZoneInfoBroker info(&kZonePacific_Galapagos);

  ZoneEraBroker era = BasicZoneProcessor::findZoneEra(info, 1984);
  assertEqual(1986, era.untilYear());

  era = BasicZoneProcessor::findZoneEra(info, 1985);
  assertEqual(1986, era.untilYear());

  era = BasicZoneProcessor::findZoneEra(info, 1986);
  assertEqual(ZoneContext::kMaxUntilYear, era.untilYear());

  era = BasicZoneProcessor::findZoneEra(info, 1987);
  assertEqual(ZoneContext::kMaxUntilYear, era.untilYear());
}

// Pacific/Galapagos transitions from simple Rule to named Rule in 1986:
test(BasicZoneProcessorTest, findLatestPriorRule) {
  // Test empty ZoneEra.Rule
  int16_t year = 1986;
  ZonePolicyBroker policy(&kZoneContext, nullptr);;
  ZoneRuleBroker rule = BasicZoneProcessor::findLatestPriorRule(policy, year);
  assertTrue(rule.isNull());

  // Policy Ecuador.
  // For year < 1992, it returns the Anchor Rule.
  year = 1800;
  policy = ZonePolicyBroker(&kZoneContext, &kZonePolicyEcuador);;
  rule = BasicZoneProcessor::findLatestPriorRule(policy, year);
  assertEqual(ZoneContext::kMinYear, rule.fromYear());

  // For year=1992, return the Anchor Rule.
  year = 1992;
  policy = ZonePolicyBroker(&kZoneContext, &kZonePolicyEcuador);;
  rule = BasicZoneProcessor::findLatestPriorRule(policy, year);
  assertEqual(ZoneContext::kMinYear, rule.fromYear());

  // For year>1992, return regular rules.
  year = 1993;
  rule = BasicZoneProcessor::findLatestPriorRule(policy, year);
  assertEqual(1992, rule.fromYear());

  year = 1994;
  rule = BasicZoneProcessor::findLatestPriorRule(policy, year);
  assertEqual(1993, rule.fromYear());

  year = 1995;
  rule = BasicZoneProcessor::findLatestPriorRule(policy, year);
  assertEqual(1993, rule.fromYear());
}

test(BasicZoneProcessorTest, priorYearOfRule) {
  ZonePolicyBroker policy(&kZoneContext, &kZonePolicyEcuador);

  int16_t year = 1995;
  assertEqual(ZoneContext::kMinYear, BasicZoneProcessor::priorYearOfRule(
      year, policy.rule(0) /*min*/));
  assertEqual(1992, BasicZoneProcessor::priorYearOfRule(
      year, policy.rule(1) /*1992*/));
  assertEqual(1993, BasicZoneProcessor::priorYearOfRule(
      year, policy.rule(2) /*1993*/));

  year = 1993;
  assertEqual(ZoneContext::kMinYear, BasicZoneProcessor::priorYearOfRule(
      year, policy.rule(0) /*min*/));
  assertEqual(1992, BasicZoneProcessor::priorYearOfRule(
      year, policy.rule(1) /*1992*/));

  // Rule[2].fromYear() is >= year, so priorYearOfRule() should not
  // be called. If it is called, it returns (incorrectly) year - 1.
  assertEqual(1992, BasicZoneProcessor::priorYearOfRule(
      year, policy.rule(2) /*1993*/));
}

test(BasicZoneProcessorTest, compareRulesBeforeYear) {
  ZonePolicyBroker policy(&kZoneContext, &kZonePolicyEcuador);

  // The last rule prior to 1995 should be 1993.
  int16_t year = 1995;
  assertLess(BasicZoneProcessor::compareRulesBeforeYear(
      year, policy.rule(0), policy.rule(1)), 0);
  assertLess(BasicZoneProcessor::compareRulesBeforeYear(
      year, policy.rule(1), policy.rule(2)), 0);

  // The last rule prior to 1993 should be 1992
  year = 1993;
  assertLess(BasicZoneProcessor::compareRulesBeforeYear(
      year, policy.rule(0), policy.rule(1)), 0);
  assertMore(BasicZoneProcessor::compareRulesBeforeYear(
      year, policy.rule(1), policy.rule(2)), 0);
}

test(BasicZoneProcessorTest, init_primitives) {
  BasicZoneProcessor zoneProcessor(&kZoneAmerica_Los_Angeles);
  zoneProcessor.mYear = 2001;
  zoneProcessor.mNumTransitions = 0;

  ZoneEraBroker priorEra = zoneProcessor.addTransitionPriorToYear(
      2001);
  assertEqual(1, zoneProcessor.mNumTransitions);
  assertEqual(-8*60*60, zoneProcessor.mTransitions[0].era.offsetSeconds());
  assertEqual("P%T", zoneProcessor.mTransitions[0].era.format());
  assertEqual(-32767, zoneProcessor.mTransitions[0].rule.fromYear());
  assertEqual(2006, zoneProcessor.mTransitions[0].rule.toYear());
  assertEqual(10, zoneProcessor.mTransitions[0].rule.inMonth());

  ZoneEraBroker currentEra = zoneProcessor.addTransitionsForYear(
      2001, priorEra);
  assertEqual(3, zoneProcessor.mNumTransitions);

  assertEqual(-8*60*60, zoneProcessor.mTransitions[1].era.offsetSeconds());
  assertEqual("P%T", zoneProcessor.mTransitions[1].era.format());
  assertEqual(1987, zoneProcessor.mTransitions[1].rule.fromYear());
  assertEqual(2006, zoneProcessor.mTransitions[1].rule.toYear());
  assertEqual(4, zoneProcessor.mTransitions[1].rule.inMonth());

  assertEqual(-8*60*60, zoneProcessor.mTransitions[2].era.offsetSeconds());
  assertEqual("P%T", zoneProcessor.mTransitions[2].era.format());
  assertEqual(-32767, zoneProcessor.mTransitions[2].rule.fromYear());
  assertEqual(2006, zoneProcessor.mTransitions[2].rule.toYear());
  assertEqual(10, zoneProcessor.mTransitions[2].rule.inMonth());

  zoneProcessor.addTransitionAfterYear(2001, currentEra);
  assertEqual(3, zoneProcessor.mNumTransitions);

  zoneProcessor.calcTransitions();
  // most recent prior is at [0]
  assertEqual(BasicZoneProcessor::kMinEpochSeconds,
      zoneProcessor.mTransitions[0].startEpochSeconds);
  assertEqual(-8*60, zoneProcessor.mTransitions[0].offsetMinutes);

  // t >= 2001-04-01 02:00 UTC-08:00 Sunday goes to PDT
  assertEqual(-7*60, zoneProcessor.mTransitions[1].offsetMinutes);
  assertEqual(
      (acetime_t) (39434400 /*relative to 2000*/
          - Epoch::daysToCurrentEpochFromInternalEpoch() * 86400),
      zoneProcessor.mTransitions[1].startEpochSeconds);

  // t >= 2001-10-28 02:00 UTC-07:00 Sunday goes to PST
  assertEqual(-8*60, zoneProcessor.mTransitions[2].offsetMinutes);
  assertEqual(
      (acetime_t) (57574800 /*relative to 2000*/
          - Epoch::daysToCurrentEpochFromInternalEpoch() * 86400),
      zoneProcessor.mTransitions[2].startEpochSeconds);
}

test(BasicZoneProcessorTest, initForLocalDate) {
  // Test using 2018-01-02. If we use 2018-01-01, the code will populate the
  // cache with transitions from 2017.
  BasicZoneProcessor zoneProcessor(&kZoneAmerica_Los_Angeles);
  LocalDate ld = LocalDate::forComponents(2018, 1, 2);
  bool ok = zoneProcessor.initForLocalDate(ld);

  assertTrue(ok);
  assertEqual(3, zoneProcessor.mNumTransitions);

  assertEqual(-8*60*60, zoneProcessor.mTransitions[0].era.offsetSeconds());
  assertEqual("P%T", zoneProcessor.mTransitions[0].era.format());
  assertEqual(2007, zoneProcessor.mTransitions[0].rule.fromYear());
  assertEqual(ZoneContext::kMaxYear,
      zoneProcessor.mTransitions[0].rule.toYear());
  assertEqual(11, zoneProcessor.mTransitions[0].rule.inMonth());

  assertEqual(-8*60*60, zoneProcessor.mTransitions[1].era.offsetSeconds());
  assertEqual("P%T", zoneProcessor.mTransitions[1].era.format());
  assertEqual(2007, zoneProcessor.mTransitions[1].rule.fromYear());
  assertEqual(ZoneContext::kMaxYear,
      zoneProcessor.mTransitions[1].rule.toYear());
  assertEqual(3, zoneProcessor.mTransitions[1].rule.inMonth());

  assertEqual(-8*60*60, zoneProcessor.mTransitions[2].era.offsetSeconds());
  assertEqual("P%T", zoneProcessor.mTransitions[2].era.format());
  assertEqual(2007, zoneProcessor.mTransitions[2].rule.fromYear());
  assertEqual(ZoneContext::kMaxYear,
      zoneProcessor.mTransitions[2].rule.toYear());
  assertEqual(11, zoneProcessor.mTransitions[2].rule.inMonth());

  assertEqual((acetime_t) BasicZoneProcessor::kMinEpochSeconds,
      zoneProcessor.mTransitions[0].startEpochSeconds);
  assertEqual(-8*60, zoneProcessor.mTransitions[0].offsetMinutes);

  // t >= 2018-03-11 02:00 UTC-08:00 Sunday goes to PDT
  assertEqual(-7*60, zoneProcessor.mTransitions[1].offsetMinutes);
  assertEqual(
      (acetime_t) (574077600 /*relative to 2000*/
          - Epoch::daysToCurrentEpochFromInternalEpoch() * 86400),
      zoneProcessor.mTransitions[1].startEpochSeconds);

  // t >= 2018-11-04 02:00 UTC-07:00 Sunday goes to PST
  assertEqual(-8*60, zoneProcessor.mTransitions[2].offsetMinutes);
  assertEqual(
      (acetime_t) (594637200 /*relative to 2000*/
          - Epoch::daysToCurrentEpochFromInternalEpoch() * 86400),
      zoneProcessor.mTransitions[2].startEpochSeconds);
}

//---------------------------------------------------------------------------
// Test public methods
//---------------------------------------------------------------------------

test(BasicZoneProcessorTest, setZoneKey) {
  BasicZoneProcessor zoneProcessor(&kZoneAmerica_Los_Angeles);
  assertEqual(zoneProcessor.mYear, LocalDate::kInvalidYear);
  zoneProcessor.initForEpochSeconds(0);
  assertNotEqual(zoneProcessor.mYear, LocalDate::kInvalidYear);

  zoneProcessor.setZoneKey((uintptr_t) &kZoneAustralia_Darwin);
  assertEqual(zoneProcessor.mYear, LocalDate::kInvalidYear);
  zoneProcessor.initForEpochSeconds(0);
  assertNotEqual(zoneProcessor.mYear, LocalDate::kInvalidYear);

  // Check that the cache remains valid if the zoneInfo does not change
  zoneProcessor.setZoneKey((uintptr_t) &kZoneAustralia_Darwin);
  assertNotEqual(zoneProcessor.mYear, LocalDate::kInvalidYear);
}

// https://www.timeanddate.com/time/zone/usa/los-angeles
test(BasicZoneProcessorTest, printNameTo) {
  BasicZoneProcessor zoneProcessor(&kZoneAmerica_Los_Angeles);

  PrintStr<32> printStr;
  zoneProcessor.printNameTo(printStr);
  assertEqual(F("America/Los_Angeles"), printStr.cstr());
  printStr.flush();
  zoneProcessor.printShortNameTo(printStr);
  assertEqual(F("Los Angeles"), printStr.cstr());
}

// Test findByEpochSeconds(). Result can be kNotFound or kExact, but
// never kGap, nor kOverlap because BasicZoneProcessor cannot detect overlap.
test(BasicZoneProcessorTest, findByEpochSeconds) {
  BasicZoneProcessor zoneProcessor(&kZoneAmerica_Los_Angeles);
  OffsetDateTime dt;
  acetime_t epochSeconds;
  FindResult result;

  // just before spring forward
  dt = OffsetDateTime::forComponents(2018, 3, 11, 1, 59, 59,
      TimeOffset::forHours(-8));
  epochSeconds = dt.toEpochSeconds();
  result = zoneProcessor.findByEpochSeconds(epochSeconds);
  assertEqual((int)result.type, (int)FindResult::kTypeExact);
  assertEqual(-8*60*60, result.stdOffsetSeconds);
  assertEqual(0*60*60, result.dstOffsetSeconds);
  assertEqual(-8*60*60, result.reqStdOffsetSeconds);
  assertEqual(0*60*60, result.reqDstOffsetSeconds);
  assertEqual("PST", result.abbrev);

  // spring forward, into the gap, but BasicZoneProcessor cannot detect it
  dt = OffsetDateTime::forComponents(2018, 3, 11, 2, 0, 1,
      TimeOffset::forHours(-8));
  epochSeconds = dt.toEpochSeconds();
  result = zoneProcessor.findByEpochSeconds(epochSeconds);
  assertEqual((int)result.type, (int)FindResult::kTypeExact);
  assertEqual(-8*60*60, result.stdOffsetSeconds);
  assertEqual(1*60*60, result.dstOffsetSeconds);
  assertEqual(-8*60*60, result.reqStdOffsetSeconds);
  assertEqual(1*60*60, result.reqDstOffsetSeconds);
  assertEqual("PDT", result.abbrev);

  // before fall back
  dt = OffsetDateTime::forComponents(2018, 11, 4, 1, 0, 0,
      TimeOffset::forHours(-7));
  epochSeconds = dt.toEpochSeconds();
  result = zoneProcessor.findByEpochSeconds(epochSeconds);
  assertEqual((int)result.type, (int)FindResult::kTypeExact);
  assertEqual(-8*60*60, result.stdOffsetSeconds);
  assertEqual(1*60*60, result.dstOffsetSeconds);
  assertEqual(-8*60*60, result.reqStdOffsetSeconds);
  assertEqual(1*60*60, result.reqDstOffsetSeconds);
  assertEqual("PDT", result.abbrev);

  // just before fall back
  dt = OffsetDateTime::forComponents(2018, 11, 4, 1, 59, 59,
      TimeOffset::forHours(-7));
  epochSeconds = dt.toEpochSeconds();
  result = zoneProcessor.findByEpochSeconds(epochSeconds);
  assertEqual((int)result.type, (int)FindResult::kTypeExact);
  assertEqual(-8*60*60, result.stdOffsetSeconds);
  assertEqual(1*60*60, result.dstOffsetSeconds);
  assertEqual(-8*60*60, result.reqStdOffsetSeconds);
  assertEqual(1*60*60, result.reqDstOffsetSeconds);
  assertEqual("PDT", result.abbrev);

  // fall back, so there is an overlap, but BasicZoneProcessor cannot detect it
  // so returns kExact
  dt = OffsetDateTime::forComponents(2018, 11, 4, 2, 0, 1,
      TimeOffset::forHours(-7));
  epochSeconds = dt.toEpochSeconds();
  result = zoneProcessor.findByEpochSeconds(epochSeconds);
  assertEqual((int)result.type, (int)FindResult::kTypeExact);
  assertEqual(-8*60*60, result.stdOffsetSeconds);
  assertEqual(0*60*60, result.dstOffsetSeconds);
  assertEqual(-8*60*60, result.reqStdOffsetSeconds);
  assertEqual(0*60*60, result.reqDstOffsetSeconds);
  assertEqual("PST", result.abbrev);

  // two hours after fall back, no overlap
  dt = OffsetDateTime::forComponents(2018, 11, 4, 3, 0, 0,
      TimeOffset::forHours(-8));
  epochSeconds = dt.toEpochSeconds();
  result = zoneProcessor.findByEpochSeconds(epochSeconds);
  assertEqual((int)result.type, (int)FindResult::kTypeExact);
  assertEqual(-8*60*60, result.stdOffsetSeconds);
  assertEqual(0*60*60, result.dstOffsetSeconds);
  assertEqual(-8*60*60, result.reqStdOffsetSeconds);
  assertEqual(0*60*60, result.reqDstOffsetSeconds);
  assertEqual("PST", result.abbrev);
}

// https://www.timeanddate.com/time/zone/south-africa/johannesburg
// No DST changes at all.
test(BasicZoneProcessorTest, kZoneAfrica_Johannesburg) {
  BasicZoneProcessor zoneProcessor(&kZoneAfrica_Johannesburg);
  OffsetDateTime dt;
  acetime_t epochSeconds;
  FindResult result;

  dt = OffsetDateTime::forComponents(2018, 1, 1, 0, 0, 0,
      TimeOffset::forHours(2));
  epochSeconds = dt.toEpochSeconds();
  result = zoneProcessor.findByEpochSeconds(epochSeconds);
  assertEqual((int)result.type, (int)FindResult::kTypeExact);
  assertEqual(2*60*60, result.stdOffsetSeconds);
  assertEqual(0*60*60, result.dstOffsetSeconds);
  assertEqual(2*60*60, result.reqStdOffsetSeconds);
  assertEqual(0*60*60, result.reqDstOffsetSeconds);
  assertEqual("SAST", result.abbrev);
}

// https://www.timeanddate.com/time/zone/australia/darwin
// No DST changes since 1944. Uses the last transition which occurred in March
// 1944.
test(BasicZoneProcessorTest, kZoneAustralia_Darwin) {
  BasicZoneProcessor zoneProcessor(&kZoneAustralia_Darwin);
  OffsetDateTime dt;
  acetime_t epochSeconds;

  dt = OffsetDateTime::forComponents(2018, 1, 1, 0, 0, 0,
      TimeOffset::forHourMinute(9, 30));
  epochSeconds = dt.toEpochSeconds();
  FindResult result = zoneProcessor.findByEpochSeconds(epochSeconds);
  assertEqual((int)result.type, (int)FindResult::kTypeExact);
  assertEqual((9*60+30)*60, result.stdOffsetSeconds);
  assertEqual(0*60*60, result.dstOffsetSeconds);
  assertEqual((9*60+30)*60, result.reqStdOffsetSeconds);
  assertEqual(0*60*60, result.reqDstOffsetSeconds);
  assertEqual("ACST", result.abbrev);
}

test(BasicZoneProcessorTest, findByEpochSeconds_outOfBounds) {
  BasicZoneProcessor zoneProcessor(&kZoneAmerica_Los_Angeles);
  EpochYearContext context(2000); // set epoch year to 2000 temporarily
  OffsetDateTime dt;
  acetime_t epochSeconds;
  FindResult result;

  ZoneContextBroker broker = ZoneContextBroker(&kZoneContext);
  assertEqual(1980, broker.startYear());
  assertEqual(2200, broker.untilYear());

  // 1970 > LocalDate::kMinYear so dt is valid, and
  dt = OffsetDateTime::forComponents(1970, 3, 11, 1, 59, 59,
      TimeOffset::forHours(-8));
  assertFalse(dt.isError());
  // 1970 is within roughly 50 years of Epoch::currentEpochYear() of 2050
  // so toEpochSeconds() still works.
  epochSeconds = dt.toEpochSeconds();
  assertNotEqual(epochSeconds, LocalDate::kInvalidEpochSeconds);
  // But 1998 < ZoneContext.startYear, so FindResult not found.
  result = zoneProcessor.findByEpochSeconds(epochSeconds);
  assertEqual((int)result.type, (int)FindResult::kTypeNotFound);

  // 10001 is beyond LocalDate::kMaxYear so should fail.
  dt = OffsetDateTime::forComponents(10001, 2, 1, 1, 0, 0,
      TimeOffset::forHours(-8));
  // 10001 > LocalDate::kMaxYear, so fails
  assertTrue(dt.isError());
  // toEpochSeconds() returns invalid seconds
  epochSeconds = dt.toEpochSeconds();
  assertEqual(epochSeconds, LocalDate::kInvalidEpochSeconds);
  // findByEpochSeconds() results NotFound for kInvalidEpochSeconds
  result = zoneProcessor.findByEpochSeconds(epochSeconds);
  assertEqual((int)result.type, (int)FindResult::kTypeNotFound);
}

//---------------------------------------------------------------------------

// Test findByLocalDateTime(). Result can be kNotFound, kExact, or kGap.
// BasicZoneProcessor cannot detect overlap so will never return kOverlap.
test(BasicZoneProcessorTest, findByLocalDateTime) {
  BasicZoneProcessor zoneProcessor(&kZoneAmerica_Los_Angeles);
  OffsetDateTime dt;
  FindResult result;

  // way before spring forward
  auto ldt = LocalDateTime::forComponents(2018, 3, 10, 1, 0, 0);
  result = zoneProcessor.findByLocalDateTime(ldt);
  assertEqual((int)result.type, (int)FindResult::kTypeExact);
  assertEqual(-8*60*60, result.stdOffsetSeconds);
  assertEqual(0*60*60, result.dstOffsetSeconds);
  assertEqual(-8*60*60, result.reqStdOffsetSeconds);
  assertEqual(0*60*60, result.reqDstOffsetSeconds);
  assertEqual("PST", result.abbrev);

  // just before spring forward
  ldt = LocalDateTime::forComponents(2018, 3, 11, 1, 59, 59);
  result = zoneProcessor.findByLocalDateTime(ldt);
  assertEqual((int)result.type, (int)FindResult::kTypeExact);
  assertEqual(-8*60*60, result.stdOffsetSeconds);
  assertEqual(0*60*60, result.dstOffsetSeconds);
  assertEqual(-8*60*60, result.reqStdOffsetSeconds);
  assertEqual(0*60*60, result.reqDstOffsetSeconds);
  assertEqual("PST", result.abbrev);

  // spring forward in the gap
  ldt = LocalDateTime::forComponents(2018, 3, 11, 2, 0, 1);
  result = zoneProcessor.findByLocalDateTime(ldt);
  assertEqual((int)result.type, (int)FindResult::kTypeGap);
  assertEqual(-8*60*60, result.stdOffsetSeconds);
  assertEqual(1*60*60, result.dstOffsetSeconds);
  assertEqual(-8*60*60, result.reqStdOffsetSeconds);
  assertEqual(1*60*60, result.reqDstOffsetSeconds);
  assertEqual("PDT", result.abbrev);

  // before fall back
  ldt = LocalDateTime::forComponents(2018, 11, 4, 1, 0, 0);
  result = zoneProcessor.findByLocalDateTime(ldt);
  assertEqual((int)result.type, (int)FindResult::kTypeExact);
  assertEqual(-8*60*60, result.stdOffsetSeconds);
  assertEqual(1*60*60, result.dstOffsetSeconds);
  assertEqual(-8*60*60, result.reqStdOffsetSeconds);
  assertEqual(1*60*60, result.reqDstOffsetSeconds);
  assertEqual("PDT", result.abbrev);

  // just before fall back
  ldt = LocalDateTime::forComponents(2018, 11, 4, 1, 59, 59);
  result = zoneProcessor.findByLocalDateTime(ldt);
  assertEqual((int)result.type, (int)FindResult::kTypeExact);
  assertEqual(-8*60*60, result.stdOffsetSeconds);
  assertEqual(1*60*60, result.dstOffsetSeconds);
  assertEqual(-8*60*60, result.reqStdOffsetSeconds);
  assertEqual(1*60*60, result.reqDstOffsetSeconds);
  assertEqual("PDT", result.abbrev);

  // fall back, so there is an overlap, but BasicZoneProcessor cannot detect it
  // so returns kExact, and selects the later of the 2 possible datetime.
  ldt = LocalDateTime::forComponents(2018, 11, 4, 2, 0, 1);
  result = zoneProcessor.findByLocalDateTime(ldt);
  assertEqual((int)result.type, (int)FindResult::kTypeExact);
  assertEqual(-8*60*60, result.stdOffsetSeconds);
  assertEqual(0*60*60, result.dstOffsetSeconds);
  assertEqual(-8*60*60, result.reqStdOffsetSeconds);
  assertEqual(0*60*60, result.reqDstOffsetSeconds);
  assertEqual("PST", result.abbrev);

  // two hours after fall back, no overlap
  ldt = LocalDateTime::forComponents(2018, 11, 4, 3, 0, 0);
  result = zoneProcessor.findByLocalDateTime(ldt);
  assertEqual((int)result.type, (int)FindResult::kTypeExact);
  assertEqual(-8*60*60, result.stdOffsetSeconds);
  assertEqual(0*60*60, result.dstOffsetSeconds);
  assertEqual(-8*60*60, result.reqStdOffsetSeconds);
  assertEqual(0*60*60, result.reqDstOffsetSeconds);
  assertEqual("PST", result.abbrev);
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
