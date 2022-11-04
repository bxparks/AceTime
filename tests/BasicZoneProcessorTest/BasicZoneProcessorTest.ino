#line 2 "BasicZoneProcessorTest.ino"

#include <AUnit.h>
#include <AceCommon.h> // PrintStr<>
#include <AceTime.h>

using ace_common::PrintStr;
using namespace ace_time;
using ace_time::internal::ZoneContext;
using ace_time::basic::compareYearMonth;
using ace_time::basic::ZoneInfo;
using ace_time::basic::ZoneEra;
using ace_time::basic::ZoneRule;
using ace_time::basic::ZonePolicy;
using ace_time::basic::ZoneInfoBroker;
using ace_time::basic::ZoneEraBroker;
using ace_time::basic::ZoneRuleBroker;
using ace_time::basic::ZonePolicyBroker;

//---------------------------------------------------------------------------
// Test zoneinfo files. Taken from Pacific/Galapagos which transitions
// from simple Rule to named Rule in 1986:
//
//# Rule  NAME    FROM    TO      TYPE    IN      ON      AT      SAVE  LETTER/S
//Rule    Ecuador 1992    only    -       Nov     28      0:00    1:00    -
//Rule    Ecuador 1993    only    -       Feb      5      0:00    0       -
//
//Zone Pacific/Galapagos  -5:58:24 -      LMT     1931 # Puerto Baquerizo Moreno
//                        -5:00   -       -05     1986
//                        -6:00   Ecuador -06/-05
//---------------------------------------------------------------------------

static const char kTzDatabaseVersion[] = "2019b";

static const ZoneContext kZoneContext = {
  1980 /*startYear*/,
  2050 /*untilYear*/,
  kTzDatabaseVersion /*tzVersion*/,
  0 /*numFragments*/,
  nullptr /*fragments*/,
};

static const ZoneRule kZoneRulesEcuador[] ACE_TIME_PROGMEM = {
  // Anchor: Rule    Ecuador    1993    only    -    Feb     5    0:00    0    -
  {
    LocalDate::kMinYear /*fromYear*/,
    LocalDate::kMinYear /*toYear*/,
    1 /*inMonth*/,
    0 /*onDayOfWeek*/,
    1 /*onDayOfMonth*/,
    0 /*atTimeCode*/,
    ZoneContext::kSuffixW /*atTimeModifier*/,
    0 /*deltaCode*/,
    '-' /*letter*/,
  },
  // Rule    Ecuador    1992    only    -    Nov    28    0:00    1:00    -
  {
    1992 /*fromYear*/,
    1992 /*toYear*/,
    11 /*inMonth*/,
    0 /*onDayOfWeek*/,
    28 /*onDayOfMonth*/,
    0 /*atTimeCode*/,
    ZoneContext::kSuffixW /*atTimeModifier*/,
    4 /*deltaCode*/,
    '-' /*letter*/,
  },
  // Rule    Ecuador    1993    only    -    Feb     5    0:00    0    -
  {
    1993 /*fromYear*/,
    1993 /*toYear*/,
    2 /*inMonth*/,
    0 /*onDayOfWeek*/,
    5 /*onDayOfMonth*/,
    0 /*atTimeCode*/,
    ZoneContext::kSuffixW /*atTimeModifier*/,
    0 /*deltaCode*/,
    '-' /*letter*/,
  },
};

static const ZonePolicy kPolicyEcuador ACE_TIME_PROGMEM = {
  kZoneRulesEcuador /*rules*/,
  nullptr /* letters */,
  3 /*numRules*/,
  0 /* numLetters */,
};

static const ZoneEra kZoneEraPacific_Galapagos[] ACE_TIME_PROGMEM = {
  //             -5:00    -    -05    1986
  {
    nullptr /*zonePolicy*/,
    "-05" /*format*/,
    -20 /*offsetCode*/,
    0 /*deltaCode*/,
    1986 /*untilYear*/,
    1 /*untilMonth*/,
    1 /*untilDay*/,
    0 /*untilTimeCode*/,
    ZoneContext::kSuffixW /*untilTimeModifier*/,
  },
  //             -6:00    Ecuador    -06/-05
  {
    &kPolicyEcuador /*zonePolicy*/,
    "-06/-05" /*format*/,
    -24 /*offsetCode*/,
    0 /*deltaCode*/,
    10000 /*untilYear*/,
    1 /*untilMonth*/,
    1 /*untilDay*/,
    0 /*untilTimeCode*/,
    ZoneContext::kSuffixW /*untilTimeModifier*/,
  },
};

static const char kZoneNamePacific_Galapagos[] ACE_TIME_PROGMEM =
    "Pacific/Galapagos";

static const ZoneInfo kZonePacific_Galapagos ACE_TIME_PROGMEM = {
  kZoneNamePacific_Galapagos /*name*/,
  0xa952f752 /*zoneId*/,
  &kZoneContext /*zoneContext*/,
  2 /*numEras*/,
  kZoneEraPacific_Galapagos /*eras*/,
};

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
  BasicZoneProcessor a(&zonedb::kZoneAmerica_Los_Angeles);
  BasicZoneProcessor b(&zonedb::kZoneAustralia_Darwin);
  assertTrue(a != b);
}

test(BasicZoneProcessorTest, setZoneKey) {
  BasicZoneProcessor zoneProcessor(&zonedb::kZoneAmerica_Los_Angeles);
  zoneProcessor.getUtcOffset(0);
  assertTrue(zoneProcessor.mIsFilled);

  zoneProcessor.setZoneKey((uintptr_t) &zonedb::kZoneAustralia_Darwin);
  assertFalse(zoneProcessor.mIsFilled);
  zoneProcessor.getUtcOffset(0);
  assertTrue(zoneProcessor.mIsFilled);

  // Check that the cache remains valid if the zoneInfo does not change
  zoneProcessor.setZoneKey((uintptr_t) &zonedb::kZoneAustralia_Darwin);
  assertTrue(zoneProcessor.mIsFilled);
}

test(BasicZoneProcessorTest, calcRuleOffsetMinutes) {
  assertEqual(0, BasicZoneProcessor::calcRuleOffsetMinutes(1, 2,
      ZoneContext::kSuffixU));
  assertEqual(1, BasicZoneProcessor::calcRuleOffsetMinutes(1, 2,
      ZoneContext::kSuffixW));
  assertEqual(2, BasicZoneProcessor::calcRuleOffsetMinutes(1, 2,
      ZoneContext::kSuffixS));
}

test(BasicZoneProcessorTest, findZoneEra) {
  ZoneInfoBroker info(&kZonePacific_Galapagos);

  ZoneEraBroker era = BasicZoneProcessor::findZoneEra(info, 1984);
  assertEqual(1986, era.untilYear());

  era = BasicZoneProcessor::findZoneEra(info, 1985);
  assertEqual(1986, era.untilYear());

  era = BasicZoneProcessor::findZoneEra(info, 1986);
  assertEqual(LocalDate::kMaxYear, era.untilYear());

  era = BasicZoneProcessor::findZoneEra(info, 1987);
  assertEqual(LocalDate::kMaxYear, era.untilYear());
}

test(BasicZoneProcessorTest, findLatestPriorRule) {
  ZonePolicyBroker policy;
  int16_t year = 1986;
  ZoneRuleBroker rule = BasicZoneProcessor::findLatestPriorRule(policy, year);
  assertTrue(rule.isNull());

  policy = ZonePolicyBroker(&kPolicyEcuador);
  year = 1992;
  rule = BasicZoneProcessor::findLatestPriorRule(policy, year);
  assertEqual(0, rule.fromYear());

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
  ZonePolicyBroker policy(&kPolicyEcuador);

  int16_t year = 1995;
  assertEqual(0, BasicZoneProcessor::priorYearOfRule(
      year, policy.rule(0) /*min*/));
  assertEqual(1992, BasicZoneProcessor::priorYearOfRule(
      year, policy.rule(1) /*1992*/));
  assertEqual(1993, BasicZoneProcessor::priorYearOfRule(
      year, policy.rule(2) /*1993*/));

  year = 1993;
  assertEqual(0, BasicZoneProcessor::priorYearOfRule(
      year, policy.rule(0) /*min*/));
  assertEqual(1992, BasicZoneProcessor::priorYearOfRule(
      year, policy.rule(1) /*1992*/));

  // Rule[2].fromYear() is >= year, so priorYearOfRule() should not
  // be called. If it is called, it returns (incorrectly) year - 1.
  assertEqual(1992, BasicZoneProcessor::priorYearOfRule(
      year, policy.rule(2) /*1993*/));
}

test(BasicZoneProcessorTest, compareRulesBeforeYear) {
  ZonePolicyBroker policy(&kPolicyEcuador);

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
  BasicZoneProcessor zoneProcessor(&zonedb::kZoneAmerica_Los_Angeles);
  zoneProcessor.mYear = 2001;
  zoneProcessor.mNumTransitions = 0;

  ZoneEraBroker priorEra = zoneProcessor.addTransitionPriorToYear(
      2001);
  assertEqual(1, zoneProcessor.mNumTransitions);
  assertEqual(-8*60, zoneProcessor.mTransitions[0].era.offsetMinutes());
  assertEqual("P%T", zoneProcessor.mTransitions[0].era.format());
  assertEqual(1967, zoneProcessor.mTransitions[0].rule.fromYear());
  assertEqual(2006, zoneProcessor.mTransitions[0].rule.toYear());
  assertEqual(10, zoneProcessor.mTransitions[0].rule.inMonth());

  ZoneEraBroker currentEra = zoneProcessor.addTransitionsForYear(
      2001, priorEra);
  assertEqual(3, zoneProcessor.mNumTransitions);

  assertEqual(-8*60, zoneProcessor.mTransitions[1].era.offsetMinutes());
  assertEqual("P%T", zoneProcessor.mTransitions[1].era.format());
  assertEqual(1987, zoneProcessor.mTransitions[1].rule.fromYear());
  assertEqual(2006, zoneProcessor.mTransitions[1].rule.toYear());
  assertEqual(4, zoneProcessor.mTransitions[1].rule.inMonth());

  assertEqual(-8*60, zoneProcessor.mTransitions[2].era.offsetMinutes());
  assertEqual("P%T", zoneProcessor.mTransitions[2].era.format());
  assertEqual(1967, zoneProcessor.mTransitions[2].rule.fromYear());
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
          - Epoch::daysToCurrentEpochFromConverterEpoch() * 86400),
      zoneProcessor.mTransitions[1].startEpochSeconds);

  // t >= 2001-10-28 02:00 UTC-07:00 Sunday goes to PST
  assertEqual(-8*60, zoneProcessor.mTransitions[2].offsetMinutes);
  assertEqual(
      (acetime_t) (57574800 /*relative to 2000*/
          - Epoch::daysToCurrentEpochFromConverterEpoch() * 86400),
      zoneProcessor.mTransitions[2].startEpochSeconds);
}

test(BasicZoneProcessorTest, init) {
  // Test using 2018-01-02. If we use 2018-01-01, the code will populate the
  // cache with transitions from 2017.
  BasicZoneProcessor zoneProcessor(&zonedb::kZoneAmerica_Los_Angeles);
  LocalDate ld = LocalDate::forComponents(2018, 1, 2);
  zoneProcessor.init(ld);

  assertEqual(3, zoneProcessor.mNumTransitions);

  assertEqual(-8*60, zoneProcessor.mTransitions[0].era.offsetMinutes());
  assertEqual("P%T", zoneProcessor.mTransitions[0].era.format());
  assertEqual(2007, zoneProcessor.mTransitions[0].rule.fromYear());
  assertEqual(ZoneRule::kMaxYear,
      zoneProcessor.mTransitions[0].rule.toYear());
  assertEqual(11, zoneProcessor.mTransitions[0].rule.inMonth());

  assertEqual(-8*60, zoneProcessor.mTransitions[1].era.offsetMinutes());
  assertEqual("P%T", zoneProcessor.mTransitions[1].era.format());
  assertEqual(2007, zoneProcessor.mTransitions[1].rule.fromYear());
  assertEqual(ZoneRule::kMaxYear,
      zoneProcessor.mTransitions[1].rule.toYear());
  assertEqual(3, zoneProcessor.mTransitions[1].rule.inMonth());

  assertEqual(-8*60, zoneProcessor.mTransitions[2].era.offsetMinutes());
  assertEqual("P%T", zoneProcessor.mTransitions[2].era.format());
  assertEqual(2007, zoneProcessor.mTransitions[2].rule.fromYear());
  assertEqual(ZoneRule::kMaxYear,
      zoneProcessor.mTransitions[2].rule.toYear());
  assertEqual(11, zoneProcessor.mTransitions[2].rule.inMonth());

  assertEqual((acetime_t) BasicZoneProcessor::kMinEpochSeconds,
      zoneProcessor.mTransitions[0].startEpochSeconds);
  assertEqual(-8*60, zoneProcessor.mTransitions[0].offsetMinutes);

  // t >= 2018-03-11 02:00 UTC-08:00 Sunday goes to PDT
  assertEqual(-7*60, zoneProcessor.mTransitions[1].offsetMinutes);
  assertEqual(
      (acetime_t) (574077600 /*relative to 2000*/
          - Epoch::daysToCurrentEpochFromConverterEpoch() * 86400),
      zoneProcessor.mTransitions[1].startEpochSeconds);

  // t >= 2018-11-04 02:00 UTC-07:00 Sunday goes to PST
  assertEqual(-8*60, zoneProcessor.mTransitions[2].offsetMinutes);
  assertEqual(
      (acetime_t) (594637200 /*relative to 2000*/
          - Epoch::daysToCurrentEpochFromConverterEpoch() * 86400),
      zoneProcessor.mTransitions[2].startEpochSeconds);
}

test(BasicZoneProcessorTest, createAbbreviation) {
  const uint8_t kDstSize = 6;
  char dst[kDstSize];

  // If no '%', deltaMinutes and letter should not matter
  BasicZoneProcessor::createAbbreviation(dst, kDstSize, "SAST", 0, '\0');
  assertEqual("SAST", dst);

  BasicZoneProcessor::createAbbreviation(dst, kDstSize, "SAST", 1*60, 'A');
  assertEqual("SAST", dst);

  // If '%', and letter is (incorrectly) set to '\0', just copy the thing
  BasicZoneProcessor::createAbbreviation(dst, kDstSize, "SA%ST", 0, '\0');
  assertEqual("SA%ST", dst);

  // If '%', then replaced with 'letter', where '-' means "no letter".
  BasicZoneProcessor::createAbbreviation(dst, kDstSize, "P%T", 0, 'S');
  assertEqual("PST", dst);

  BasicZoneProcessor::createAbbreviation(dst, kDstSize, "P%T", 1*60, 'D');
  assertEqual("PDT", dst);

  BasicZoneProcessor::createAbbreviation(dst, kDstSize, "P%T", 0, '-');
  assertEqual("PT", dst);

  // If '/', then deltaMinutes selects the first or second component.
  BasicZoneProcessor::createAbbreviation(dst, kDstSize, "GMT/BST", 0, '-');
  assertEqual("GMT", dst);

  BasicZoneProcessor::createAbbreviation(dst, kDstSize, "GMT/BST", 0, '\0');
  assertEqual("GMT", dst);

  BasicZoneProcessor::createAbbreviation(dst, kDstSize, "GMT/BST", 1*60, '-');
  assertEqual("BST", dst);

  BasicZoneProcessor::createAbbreviation(dst, kDstSize, "GMT/BST", 1*60, '\0');
  assertEqual("BST", dst);

  // test truncation to kDstSize
  BasicZoneProcessor::createAbbreviation(dst, kDstSize, "P%T3456", 1*60, 'D');
  assertEqual("PDT34", dst);
}

//---------------------------------------------------------------------------
// Test public methods
//---------------------------------------------------------------------------

// https://www.timeanddate.com/time/zone/usa/los-angeles
test(BasicZoneProcessorTest, kZoneAmerica_Los_Angeles) {
  BasicZoneProcessor zoneProcessor(&zonedb::kZoneAmerica_Los_Angeles);

  PrintStr<32> printStr;
  zoneProcessor.printNameTo(printStr);
  assertEqual(F("America/Los_Angeles"), printStr.cstr());
  printStr.flush();
  zoneProcessor.printShortNameTo(printStr);
  assertEqual(F("Los Angeles"), printStr.cstr());

  OffsetDateTime dt;
  acetime_t epochSeconds;

  dt = OffsetDateTime::forComponents(2018, 3, 11, 1, 59, 59,
      TimeOffset::forHours(-8));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-8*60, zoneProcessor.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("PST", zoneProcessor.getAbbrev(epochSeconds));
  assertTrue(zoneProcessor.getDeltaOffset(epochSeconds).isZero());

  dt = OffsetDateTime::forComponents(2018, 3, 11, 2, 0, 0,
      TimeOffset::forHours(-8));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-7*60, zoneProcessor.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("PDT", zoneProcessor.getAbbrev(epochSeconds));
  assertFalse(zoneProcessor.getDeltaOffset(epochSeconds).isZero());

  dt = OffsetDateTime::forComponents(2018, 11, 4, 1, 0, 0,
      TimeOffset::forHours(-7));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-7*60, zoneProcessor.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("PDT", zoneProcessor.getAbbrev(epochSeconds));
  assertFalse(zoneProcessor.getDeltaOffset(epochSeconds).isZero());

  dt = OffsetDateTime::forComponents(2018, 11, 4, 1, 59, 59,
      TimeOffset::forHours(-7));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-7*60, zoneProcessor.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("PDT", zoneProcessor.getAbbrev(epochSeconds));
  assertFalse(zoneProcessor.getDeltaOffset(epochSeconds).isZero());

  dt = OffsetDateTime::forComponents(2018, 11, 4, 2, 0, 0,
      TimeOffset::forHours(-7));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(-8*60, zoneProcessor.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("PST", zoneProcessor.getAbbrev(epochSeconds));
  assertTrue(zoneProcessor.getDeltaOffset(epochSeconds).isZero());
}

// https://www.timeanddate.com/time/zone/south-africa/johannesburg
// No DST changes at all.
test(BasicZoneProcessorTest, kZoneAfrica_Johannesburg) {
  BasicZoneProcessor zoneProcessor(&zonedb::kZoneAfrica_Johannesburg);
  OffsetDateTime dt;
  acetime_t epochSeconds;

  dt = OffsetDateTime::forComponents(2018, 1, 1, 0, 0, 0,
      TimeOffset::forHours(2));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(2*60, zoneProcessor.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("SAST", zoneProcessor.getAbbrev(epochSeconds));
  assertTrue(zoneProcessor.getDeltaOffset(epochSeconds).isZero());
}

// https://www.timeanddate.com/time/zone/australia/darwin
// No DST changes since 1944. Uses the last transition which occurred in March
// 1944.
test(BasicZoneProcessorTest, kZoneAustralia_Darwin) {
  BasicZoneProcessor zoneProcessor(&zonedb::kZoneAustralia_Darwin);
  OffsetDateTime dt;
  acetime_t epochSeconds;

  dt = OffsetDateTime::forComponents(2018, 1, 1, 0, 0, 0,
      TimeOffset::forHourMinute(9, 30));
  epochSeconds = dt.toEpochSeconds();
  assertEqual(9*60+30, zoneProcessor.getUtcOffset(epochSeconds).toMinutes());
  assertEqual("ACST", zoneProcessor.getAbbrev(epochSeconds));
  assertTrue(zoneProcessor.getDeltaOffset(epochSeconds).isZero());
}

test(BasicZoneProcessorTest, kZoneAmerica_Los_Angeles_outOfBounds) {
  BasicZoneProcessor zoneProcessor(&zonedb::kZoneAmerica_Los_Angeles);
  OffsetDateTime dt;
  acetime_t epochSeconds;

  assertEqual(2000, zonedb::kZoneContext.startYear);
  assertEqual(10000, zonedb::kZoneContext.untilYear);

  // 1998 > LocalDate::kMinYear so dt is valid, and
  dt = OffsetDateTime::forComponents(1998, 3, 11, 1, 59, 59,
      TimeOffset::forHours(-8));
  assertFalse(dt.isError());
  // 1998 is within roughly 50 years of Epoch::currentEpochYear() of 2050
  // so toEpochSeconds() still works.
  epochSeconds = dt.toEpochSeconds();
  assertNotEqual(epochSeconds, LocalDate::kInvalidEpochSeconds);
  // 1998 < ZoneContext.startYear, so getUtcOffset() fails
  assertTrue(zoneProcessor.getUtcOffset(epochSeconds).isError());
  // 1998 < ZoneContext.startYear, so getDeltaOffset() fails
  assertTrue(zoneProcessor.getDeltaOffset(epochSeconds).isError());
  // getAbbrev() returns "" on lookup failure
  assertEqual("", zoneProcessor.getAbbrev(epochSeconds));

  // 10001 is beyond LocalDate::kMaxYear so should fail.
  dt = OffsetDateTime::forComponents(10001, 2, 1, 1, 0, 0,
      TimeOffset::forHours(-8));
  // 10001 > LocalDate::kMaxYear, so fails
  assertTrue(dt.isError());
  // toEpochSeconds() returns invalid seconds
  epochSeconds = dt.toEpochSeconds();
  assertEqual(epochSeconds, LocalDate::kInvalidEpochSeconds);
  // getUtcOffset() fails for kInvalidEpochSeconds
  assertTrue(zoneProcessor.getUtcOffset(epochSeconds).isError());
  // getDeltaOffset() fails for kInvalidEpochSeconds
  assertTrue(zoneProcessor.getDeltaOffset(epochSeconds).isError());
  // getAbbrev() returns "" on lookup failure
  assertEqual("", zoneProcessor.getAbbrev(epochSeconds));
}

//---------------------------------------------------------------------------

void setup() {
#if ! defined(EPOXY_DUINO)
  delay(1000); // wait to prevent garbage on SERIAL_PORT_MONITOR
#endif
  SERIAL_PORT_MONITOR.begin(115200);
  while (!SERIAL_PORT_MONITOR); // Leonardo/Micro
}

void loop() {
  aunit::TestRunner::run();
}
