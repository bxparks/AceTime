#line 2 "BasicZoneProcessorTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time;

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

static const basic::ZoneContext kZoneContext = {
  1980 /*startYear*/,
  2050 /*untilYear*/,
  kTzDatabaseVersion /*tzVersion*/,
  0 /*numFragments*/,
  nullptr /*fragments*/,
};

static const basic::ZoneRule kZoneRulesEcuador[] ACE_TIME_PROGMEM = {
  // Anchor: Rule    Ecuador    1993    only    -    Feb     5    0:00    0    -
  {
    -127 /*fromYearTiny*/,
    -127 /*toYearTiny*/,
    1 /*inMonth*/,
    0 /*onDayOfWeek*/,
    1 /*onDayOfMonth*/,
    0 /*atTimeCode*/,
    basic::ZoneContext::kSuffixW /*atTimeModifier*/,
    0 /*deltaCode*/,
    '-' /*letter*/,
  },
  // Rule    Ecuador    1992    only    -    Nov    28    0:00    1:00    -
  {
    -8 /*fromYearTiny*/,
    -8 /*toYearTiny*/,
    11 /*inMonth*/,
    0 /*onDayOfWeek*/,
    28 /*onDayOfMonth*/,
    0 /*atTimeCode*/,
    basic::ZoneContext::kSuffixW /*atTimeModifier*/,
    4 /*deltaCode*/,
    '-' /*letter*/,
  },
  // Rule    Ecuador    1993    only    -    Feb     5    0:00    0    -
  {
    -7 /*fromYearTiny*/,
    -7 /*toYearTiny*/,
    2 /*inMonth*/,
    0 /*onDayOfWeek*/,
    5 /*onDayOfMonth*/,
    0 /*atTimeCode*/,
    basic::ZoneContext::kSuffixW /*atTimeModifier*/,
    0 /*deltaCode*/,
    '-' /*letter*/,
  },
};

static const basic::ZonePolicy kPolicyEcuador ACE_TIME_PROGMEM = {
  kZoneRulesEcuador /*rules*/,
  nullptr /* letters */,
  3 /*numRules*/,
  0 /* numLetters */,
};

static const basic::ZoneEra kZoneEraPacific_Galapagos[] ACE_TIME_PROGMEM = {
  //             -5:00    -    -05    1986
  {
    nullptr /*zonePolicy*/,
    "-05" /*format*/,
    -20 /*offsetCode*/,
    0 /*deltaCode*/,
    -14 /*untilYearTiny*/,
    1 /*untilMonth*/,
    1 /*untilDay*/,
    0 /*untilTimeCode*/,
    basic::ZoneContext::kSuffixW /*untilTimeModifier*/,
  },
  //             -6:00    Ecuador    -06/-05
  {
    &kPolicyEcuador /*zonePolicy*/,
    "-06/-05" /*format*/,
    -24 /*offsetCode*/,
    0 /*deltaCode*/,
    127 /*untilYearTiny*/,
    1 /*untilMonth*/,
    1 /*untilDay*/,
    0 /*untilTimeCode*/,
    basic::ZoneContext::kSuffixW /*untilTimeModifier*/,
  },
};

static const char kZoneNamePacific_Galapagos[] ACE_TIME_PROGMEM =
    "Pacific/Galapagos";

static const basic::ZoneInfo kZonePacific_Galapagos ACE_TIME_PROGMEM = {
  kZoneNamePacific_Galapagos /*name*/,
  0xa952f752 /*zoneId*/,
  &kZoneContext /*zoneContext*/,
  2 /*numEras*/,
  kZoneEraPacific_Galapagos /*eras*/,
};

//---------------------------------------------------------------------------
// BasicZoneProcessor: test private methods
//---------------------------------------------------------------------------

test(BasicZoneProcessorTest, operatorEqualEqual) {
  BasicZoneProcessor a(&zonedb::kZoneAmerica_Los_Angeles);
  BasicZoneProcessor b(&zonedb::kZoneAustralia_Darwin);
  assertTrue(a != b);
}

test(BasicZoneProcessorTest, setZoneInfo) {
  BasicZoneProcessor zoneInfo(&zonedb::kZoneAmerica_Los_Angeles);
  zoneInfo.getUtcOffset(0);
  assertTrue(zoneInfo.mIsFilled);

  zoneInfo.setZoneInfo(&zonedb::kZoneAustralia_Darwin);
  assertFalse(zoneInfo.mIsFilled);
  zoneInfo.getUtcOffset(0);
  assertTrue(zoneInfo.mIsFilled);

  // Check that the cache remains valid if the zoneInfo does not change
  zoneInfo.setZoneInfo(&zonedb::kZoneAustralia_Darwin);
  assertTrue(zoneInfo.mIsFilled);
}

test(BasicZoneProcessorTest, calcStartDayOfMonth) {
  // 2018-11, Sun>=1
  basic::MonthDay monthDay = BasicZoneProcessor::calcStartDayOfMonth(
      2018, 11, LocalDate::kSunday, 1);
  assertEqual(11, monthDay.month);
  assertEqual(4, monthDay.day);

  // 2018-11, lastSun
  monthDay = BasicZoneProcessor::calcStartDayOfMonth(
      2018, 11, LocalDate::kSunday, 0);
  assertEqual(11, monthDay.month);
  assertEqual(25, monthDay.day);

  // 2018-11, Sun>=30, should shift to 2018-12-2
  monthDay = BasicZoneProcessor::calcStartDayOfMonth(
      2018, 11, LocalDate::kSunday, 30);
  assertEqual(12, monthDay.month);
  assertEqual(2, monthDay.day);

  // 2018-11, Mon<=7
  monthDay = BasicZoneProcessor::calcStartDayOfMonth(
      2018, 11, LocalDate::kMonday, -7);
  assertEqual(11, monthDay.month);
  assertEqual(5, monthDay.day);

  // 2018-11, Mon<=1, shifts back into October
  monthDay = BasicZoneProcessor::calcStartDayOfMonth(
      2018, 11, LocalDate::kMonday, -1);
  assertEqual(10, monthDay.month);
  assertEqual(29, monthDay.day);

  // 2018-03, Thu>=9
  monthDay = BasicZoneProcessor::calcStartDayOfMonth(
      2018, 3, LocalDate::kThursday, 9);
  assertEqual(3, monthDay.month);
  assertEqual(15, monthDay.day);

  // 2018-03-30 exactly
  monthDay = BasicZoneProcessor::calcStartDayOfMonth(2018, 3, 0, 30);
  assertEqual(3, monthDay.month);
  assertEqual(30, monthDay.day);
}

test(BasicZoneProcessorTest, calcRuleOffsetMinutes) {
  assertEqual(0, BasicZoneProcessor::calcRuleOffsetMinutes(1, 2,
      basic::ZoneContext::kSuffixU));
  assertEqual(1, BasicZoneProcessor::calcRuleOffsetMinutes(1, 2,
      basic::ZoneContext::kSuffixW));
  assertEqual(2, BasicZoneProcessor::calcRuleOffsetMinutes(1, 2,
      basic::ZoneContext::kSuffixS));
}

test(BasicZoneProcessorTest, findZoneEra) {
  basic::ZoneInfoBroker info(&kZonePacific_Galapagos);

  basic::ZoneEraBroker era = BasicZoneProcessor::findZoneEra(info, 1984-2000);
  assertEqual(1986-2000, era.untilYearTiny());

  era = BasicZoneProcessor::findZoneEra(info, 1985-2000);
  assertEqual(1986-2000, era.untilYearTiny());

  era = BasicZoneProcessor::findZoneEra(info, 1986-2000);
  assertEqual(LocalDate::kMaxYearTiny, era.untilYearTiny());

  era = BasicZoneProcessor::findZoneEra(info, 1987-2000);
  assertEqual(LocalDate::kMaxYearTiny, era.untilYearTiny());
}

test(BasicZoneProcessorTest, findLatestPriorRule) {
  basic::ZonePolicyBroker policy;
  int8_t yearTiny = 1986-2000;
  basic::ZoneRuleBroker rule = BasicZoneProcessor::findLatestPriorRule(
      policy, yearTiny);
  assertTrue(rule.isNull());

  policy = basic::ZonePolicyBroker(&kPolicyEcuador);
  yearTiny = 1992-2000;
  rule = BasicZoneProcessor::findLatestPriorRule(policy, yearTiny);
  assertEqual(-127, rule.fromYearTiny());

  yearTiny = 1993-2000;
  rule = BasicZoneProcessor::findLatestPriorRule(policy, yearTiny);
  assertEqual(1992-2000, rule.fromYearTiny());

  yearTiny = 1994-2000;
  rule = BasicZoneProcessor::findLatestPriorRule(policy, yearTiny);
  assertEqual(1993-2000, rule.fromYearTiny());

  yearTiny = 1995-2000;
  rule = BasicZoneProcessor::findLatestPriorRule(policy, yearTiny);
  assertEqual(1993-2000, rule.fromYearTiny());
}

test(BasicZoneProcessorTest, priorYearOfRule) {
  basic::ZonePolicyBroker policy(&kPolicyEcuador);

  int8_t yearTiny = 1995-2000;
  assertEqual(1873-2000, BasicZoneProcessor::priorYearOfRule(
      yearTiny, policy.rule(0) /*min*/));
  assertEqual(1992-2000, BasicZoneProcessor::priorYearOfRule(
      yearTiny, policy.rule(1) /*1992*/));
  assertEqual(1993-2000, BasicZoneProcessor::priorYearOfRule(
      yearTiny, policy.rule(2) /*1993*/));

  yearTiny = 1993-2000;
  assertEqual(1873-2000, BasicZoneProcessor::priorYearOfRule(
      yearTiny, policy.rule(0) /*min*/));
  assertEqual(1992-2000, BasicZoneProcessor::priorYearOfRule(
      yearTiny, policy.rule(1) /*1992*/));

  // Rule[2].fromYearTiny() is >= yearTiny, so priorYearOfRule() should not
  // be called. If it is called, it returns (incorrectly) yearTiny - 1.
  assertEqual(1992-2000, BasicZoneProcessor::priorYearOfRule(
      yearTiny, policy.rule(2) /*1993*/));
}

test(BasicZoneProcessorTest, compareRulesBeforeYear) {
  basic::ZonePolicyBroker policy(&kPolicyEcuador);

  // The last rule prior to 1995 should be 1993.
  int8_t yearTiny = 1995-2000;
  assertLess(BasicZoneProcessor::compareRulesBeforeYear(
      yearTiny, policy.rule(0), policy.rule(1)), 0);
  assertLess(BasicZoneProcessor::compareRulesBeforeYear(
      yearTiny, policy.rule(1), policy.rule(2)), 0);

  // The last rule prior to 1993 should be 1992
  yearTiny = 1993-2000;
  assertLess(BasicZoneProcessor::compareRulesBeforeYear(
      yearTiny, policy.rule(0), policy.rule(1)), 0);
  assertMore(BasicZoneProcessor::compareRulesBeforeYear(
      yearTiny, policy.rule(1), policy.rule(2)), 0);
}

test(BasicZoneProcessorTest, init_primitives) {
  BasicZoneProcessor zoneProcessor(&zonedb::kZoneAmerica_Los_Angeles);
  zoneProcessor.mYearTiny = 2001-2000;
  zoneProcessor.mNumTransitions = 0;

  basic::ZoneEraBroker priorEra = zoneProcessor.addTransitionPriorToYear(
      2001-2000);
  assertEqual(1, zoneProcessor.mNumTransitions);
  assertEqual(-8*60, zoneProcessor.mTransitions[0].era.offsetMinutes());
  assertEqual("P%T", zoneProcessor.mTransitions[0].era.format());
  assertEqual(1967-2000, zoneProcessor.mTransitions[0].rule.fromYearTiny());
  assertEqual(2006-2000, zoneProcessor.mTransitions[0].rule.toYearTiny());
  assertEqual(10, zoneProcessor.mTransitions[0].rule.inMonth());

  basic::ZoneEraBroker currentEra = zoneProcessor.addTransitionsForYear(
      2001-2000, priorEra);
  assertEqual(3, zoneProcessor.mNumTransitions);

  assertEqual(-8*60, zoneProcessor.mTransitions[1].era.offsetMinutes());
  assertEqual("P%T", zoneProcessor.mTransitions[1].era.format());
  assertEqual(1987-2000, zoneProcessor.mTransitions[1].rule.fromYearTiny());
  assertEqual(2006-2000, zoneProcessor.mTransitions[1].rule.toYearTiny());
  assertEqual(4, zoneProcessor.mTransitions[1].rule.inMonth());

  assertEqual(-8*60, zoneProcessor.mTransitions[2].era.offsetMinutes());
  assertEqual("P%T", zoneProcessor.mTransitions[2].era.format());
  assertEqual(1967-2000, zoneProcessor.mTransitions[2].rule.fromYearTiny());
  assertEqual(2006-2000, zoneProcessor.mTransitions[2].rule.toYearTiny());
  assertEqual(10, zoneProcessor.mTransitions[2].rule.inMonth());

  zoneProcessor.addTransitionAfterYear(2001-2000, currentEra);
  assertEqual(3, zoneProcessor.mNumTransitions);

  zoneProcessor.calcTransitions();
  // most recent prior is at [0]
  assertEqual(BasicZoneProcessor::kMinEpochSeconds,
      zoneProcessor.mTransitions[0].startEpochSeconds);
  assertEqual(-8*60, zoneProcessor.mTransitions[0].offsetMinutes);

  // t >= 2001-04-01 02:00 UTC-08:00 Sunday goes to PDT
  assertEqual(-7*60, zoneProcessor.mTransitions[1].offsetMinutes);
  assertEqual((acetime_t) 39434400,
      zoneProcessor.mTransitions[1].startEpochSeconds);

  // t >= 2001-10-28 02:00 UTC-07:00 Sunday goes to PST
  assertEqual(-8*60, zoneProcessor.mTransitions[2].offsetMinutes);
  assertEqual((acetime_t) 57574800,
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
  assertEqual(2007-2000, zoneProcessor.mTransitions[0].rule.fromYearTiny());
  assertEqual(basic::ZoneRule::kMaxYearTiny,
      zoneProcessor.mTransitions[0].rule.toYearTiny());
  assertEqual(11, zoneProcessor.mTransitions[0].rule.inMonth());

  assertEqual(-8*60, zoneProcessor.mTransitions[1].era.offsetMinutes());
  assertEqual("P%T", zoneProcessor.mTransitions[1].era.format());
  assertEqual(2007-2000, zoneProcessor.mTransitions[1].rule.fromYearTiny());
  assertEqual(basic::ZoneRule::kMaxYearTiny,
      zoneProcessor.mTransitions[1].rule.toYearTiny());
  assertEqual(3, zoneProcessor.mTransitions[1].rule.inMonth());

  assertEqual(-8*60, zoneProcessor.mTransitions[2].era.offsetMinutes());
  assertEqual("P%T", zoneProcessor.mTransitions[2].era.format());
  assertEqual(2007-2000, zoneProcessor.mTransitions[2].rule.fromYearTiny());
  assertEqual(basic::ZoneRule::kMaxYearTiny,
      zoneProcessor.mTransitions[2].rule.toYearTiny());
  assertEqual(11, zoneProcessor.mTransitions[2].rule.inMonth());

  assertEqual((acetime_t) BasicZoneProcessor::kMinEpochSeconds,
      zoneProcessor.mTransitions[0].startEpochSeconds);
  assertEqual(-8*60, zoneProcessor.mTransitions[0].offsetMinutes);

  // t >= 2018-03-11 02:00 UTC-08:00 Sunday goes to PDT
  assertEqual(-7*60, zoneProcessor.mTransitions[1].offsetMinutes);
  assertEqual((acetime_t) 574077600,
      zoneProcessor.mTransitions[1].startEpochSeconds);

  // t >= 2018-11-04 02:00 UTC-07:00 Sunday goes to PST
  assertEqual(-8*60, zoneProcessor.mTransitions[2].offsetMinutes);
  assertEqual((acetime_t) 594637200,
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

  assertEqual(2000, zonedb::kZoneAmerica_Los_Angeles.zoneContext->startYear);
  assertEqual(2050, zonedb::kZoneAmerica_Los_Angeles.zoneContext->untilYear);

  dt = OffsetDateTime::forComponents(1998, 3, 11, 1, 59, 59,
      TimeOffset::forHours(-8));
  epochSeconds = dt.toEpochSeconds();
  assertTrue(zoneProcessor.getUtcOffset(epochSeconds).isError());
  assertTrue(zoneProcessor.getDeltaOffset(epochSeconds).isError());
  assertEqual("", zoneProcessor.getAbbrev(epochSeconds));

  dt = OffsetDateTime::forComponents(2051, 2, 1, 1, 0, 0,
      TimeOffset::forHours(-8));
  epochSeconds = dt.toEpochSeconds();
  assertTrue(zoneProcessor.getUtcOffset(epochSeconds).isError());
  assertTrue(zoneProcessor.getDeltaOffset(epochSeconds).isError());
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
  TestRunner::run();
}
