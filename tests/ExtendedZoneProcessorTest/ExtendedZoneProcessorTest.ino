#line 2 "ExtendedZoneProcessorTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time;
using namespace ace_time::extended;

// --------------------------------------------------------------------------
// A simplified version of America/Los_Angeles, using only simple ZoneEras
// (i.e. no references to a ZonePolicy). Valid only for 2018.
// --------------------------------------------------------------------------

static const ZoneContext kZoneContext = {
  2000 /*startYear*/,
  2020 /*untilYear*/,
  "testing" /*tzVersion*/,
};

// Create simplified ZoneEras which approximate America/Los_Angeles
static const ZoneEra kZoneEraAlmostLosAngeles[] ACE_TIME_PROGMEM = {
  {
    nullptr,
    "PST" /*format*/,
    -32 /*offsetCode*/,
    0 /*deltaCode*/,
    19 /*untilYearTiny*/,
    3 /*untilMonth*/,
    10 /*untilDay*/,
    2*4 /*untilTimeCode*/,
    ZoneContext::TIME_MODIFIER_W /*untilTimeModifier*/
  },
  {
    nullptr,
    "PDT" /*format*/,
    -32 /*offsetCode*/,
    4 /*deltaCode*/,
    19 /*untilYearTiny*/,
    11 /*untilMonth*/,
    3 /*untilDay*/,
    2*4 /*untilTimeCode*/,
    ZoneContext::TIME_MODIFIER_W /*untilTimeModifier*/
  },
  {
    nullptr,
    "PST" /*format*/,
    -32 /*offsetCode*/,
    0 /*deltaCode*/,
    20 /*untilYearTiny*/,
    3 /*untilMonth*/,
    8 /*untilDay*/,
    2*4 /*untilTimeCode*/,
    ZoneContext::TIME_MODIFIER_W /*untilTimeModifier*/
  },
};

static const ZoneInfo kZoneAlmostLosAngeles ACE_TIME_PROGMEM = {
  "Almost_Los_Angeles" /*name*/,
  0x70166020 /*zoneId*/,
  &::kZoneContext /*zoneContext*/,
  7 /*transitionBufSize*/,
  3 /*numEras*/,
  kZoneEraAlmostLosAngeles /*eras*/,
};

// --------------------------------------------------------------------------
// A real ZoneInfo for America/Los_Angeles. Taken from zonedbx/zone_infos.cpp.
// --------------------------------------------------------------------------

static const ZoneRule kZoneRulesTestUS[] ACE_TIME_PROGMEM = {
  // Rule    US    1967    2006    -    Oct    lastSun    2:00    0    S
  {
    -33 /*fromYearTiny*/,
    6 /*toYearTiny*/,
    10 /*inMonth*/,
    7 /*onDayOfWeek*/,
    0 /*onDayOfMonth*/,
    8 /*atTimeCode*/,
    ZoneContext::TIME_MODIFIER_W /*atTimeModifier*/,
    0 /*deltaCode*/,
    'S' /*letter*/,
  },
  // Rule    US    1976    1986    -    Apr    lastSun    2:00    1:00    D
  {
    -24 /*fromYearTiny*/,
    -14 /*toYearTiny*/,
    4 /*inMonth*/,
    7 /*onDayOfWeek*/,
    0 /*onDayOfMonth*/,
    8 /*atTimeCode*/,
    ZoneContext::TIME_MODIFIER_W /*atTimeModifier*/,
    4 /*deltaCode*/,
    'D' /*letter*/,
  },
  // Rule    US    1987    2006    -    Apr    Sun>=1    2:00    1:00    D
  {
    -13 /*fromYearTiny*/,
    6 /*toYearTiny*/,
    4 /*inMonth*/,
    7 /*onDayOfWeek*/,
    1 /*onDayOfMonth*/,
    8 /*atTimeCode*/,
    ZoneContext::TIME_MODIFIER_W /*atTimeModifier*/,
    4 /*deltaCode*/,
    'D' /*letter*/,
  },
  // Rule    US    2007    max    -    Mar    Sun>=8    2:00    1:00    D
  {
    7 /*fromYearTiny*/,
    126 /*toYearTiny*/,
    3 /*inMonth*/,
    7 /*onDayOfWeek*/,
    8 /*onDayOfMonth*/,
    8 /*atTimeCode*/,
    ZoneContext::TIME_MODIFIER_W /*atTimeModifier*/,
    4 /*deltaCode*/,
    'D' /*letter*/,
  },
  // Rule    US    2007    max    -    Nov    Sun>=1    2:00    0    S
  {
    7 /*fromYearTiny*/,
    126 /*toYearTiny*/,
    11 /*inMonth*/,
    7 /*onDayOfWeek*/,
    1 /*onDayOfMonth*/,
    8 /*atTimeCode*/,
    ZoneContext::TIME_MODIFIER_W /*atTimeModifier*/,
    0 /*deltaCode*/,
    'S' /*letter*/,
  },

};

static const ZonePolicy kPolicyTestUS ACE_TIME_PROGMEM = {
  kZoneRulesTestUS /*rules*/,
  nullptr /* letters */,
  5 /*numRules*/,
  0 /* numLetters */,
};

static const ZoneEra kZoneEraTestLos_Angeles[] ACE_TIME_PROGMEM = {
  //             -8:00    US    P%sT
  {
    &kPolicyTestUS /*zonePolicy*/,
    "P%T" /*format*/,
    -32 /*offsetCode*/,
    0 /*deltaCode*/,
    127 /*untilYearTiny*/,
    1 /*untilMonth*/,
    1 /*untilDay*/,
    0 /*untilTimeCode*/,
    ZoneContext::TIME_MODIFIER_W /*untilTimeModifier*/,
  },

};

static const ZoneInfo kZoneTestLos_Angeles ACE_TIME_PROGMEM = {
  "America/Los_Angeles" /*name*/,
  0xb7f7e8f2 /*zoneId*/,
  &::kZoneContext /*zoneContext*/,
  7 /*transitionBufSize*/,
  1 /*numEras*/,
  kZoneEraTestLos_Angeles /*eras*/,
};

// --------------------------------------------------------------------------
// ExtendedZoneProcessor: test private methods
// --------------------------------------------------------------------------

test(ExtendedZoneProcessorTest, tzVersion) {
  assertEqual("2019b", zonedbx::kTzDatabaseVersion);
}

static const ZoneEra era ACE_TIME_PROGMEM =
    {nullptr, "", 0, 0, 0, 1, 2, 12, ZoneContext::TIME_MODIFIER_W};

test(ExtendedZoneProcessorTest, compareEraToYearMonth) {
  assertEqual(1, ExtendedZoneProcessor::compareEraToYearMonth(
      extended::ZoneEraBroker(&era), 0, 1));
  assertEqual(1, ExtendedZoneProcessor::compareEraToYearMonth(
      extended::ZoneEraBroker(&era), 0, 1));
  assertEqual(-1, ExtendedZoneProcessor::compareEraToYearMonth(
      extended::ZoneEraBroker(&era), 0, 2));
  assertEqual(-1, ExtendedZoneProcessor::compareEraToYearMonth(
      extended::ZoneEraBroker(&era), 0, 3));
}

static const ZoneEra era2 ACE_TIME_PROGMEM =
    {nullptr, "", 0, 0, 0, 1, 0, 0, ZoneContext::TIME_MODIFIER_W};

test(ExtendedZoneProcessorTest, compareEraToYearMonth2) {
  assertEqual(0, ExtendedZoneProcessor::compareEraToYearMonth(
      extended::ZoneEraBroker(&era2), 0, 1));
}

// UNTIL = 2000-01-02 3:00
static const ZoneEra prev ACE_TIME_PROGMEM =
    {nullptr, "", 0, 0, 0, 1, 2, 3, ZoneContext::TIME_MODIFIER_W};

// UNTIL = 2002-03-04 5:00
static const ZoneEra era3 ACE_TIME_PROGMEM =
    {nullptr, "", 0, 0, 2, 3, 4, 5, ZoneContext::TIME_MODIFIER_W};

test(ExtendedZoneProcessorTest, createMatch) {
  YearMonthTuple startYm = {0, 12};
  YearMonthTuple untilYm = {1, 2};
  ZoneMatch match = ExtendedZoneProcessor::createMatch(
      extended::ZoneEraBroker(&prev), extended::ZoneEraBroker(&era3),
      startYm, untilYm);
  assertTrue((match.startDateTime == DateTuple{0, 12, 1, 0,
      ZoneContext::TIME_MODIFIER_W}));
  assertTrue((match.untilDateTime == DateTuple{1, 2, 1, 0,
      ZoneContext::TIME_MODIFIER_W}));
  assertTrue(&era3 == match.era.zoneEra());

  startYm = {-1, 12};
  untilYm = {3, 2};
  match = ExtendedZoneProcessor::createMatch(
      extended::ZoneEraBroker(&prev), extended::ZoneEraBroker(&era3),
      startYm, untilYm);
  assertTrue((match.startDateTime == DateTuple{0, 1, 2, 15*3,
      ZoneContext::TIME_MODIFIER_W}));
  assertTrue((match.untilDateTime == DateTuple{2, 3, 4, 15*5,
      ZoneContext::TIME_MODIFIER_W}));
  assertTrue(&era3 == match.era.zoneEra());
}

test(ExtendedZoneProcessorTest, findMatches_simple) {
  YearMonthTuple startYm = {18, 12};
  YearMonthTuple untilYm = {20, 2};
  const uint8_t kMaxMaches = 4;
  ZoneMatch matches[kMaxMaches];
  uint8_t numMatches = ExtendedZoneProcessor::findMatches(
      extended::ZoneInfoBroker(&kZoneAlmostLosAngeles), startYm, untilYm,
      matches, kMaxMaches);
  assertEqual(3, numMatches);

  assertTrue((matches[0].startDateTime == DateTuple{18, 12, 1, 0,
      ZoneContext::TIME_MODIFIER_W}));
  assertTrue((matches[0].untilDateTime == DateTuple{19, 3, 10, 15*8,
      ZoneContext::TIME_MODIFIER_W}));
  assertTrue(&kZoneEraAlmostLosAngeles[0] == matches[0].era.zoneEra());

  assertTrue((matches[1].startDateTime == DateTuple{19, 3, 10, 15*8,
      ZoneContext::TIME_MODIFIER_W}));
  assertTrue((matches[1].untilDateTime == DateTuple{19, 11, 3, 15*8,
      ZoneContext::TIME_MODIFIER_W}));
  assertTrue(&kZoneEraAlmostLosAngeles[1] == matches[1].era.zoneEra());

  assertTrue((matches[2].startDateTime == DateTuple{19, 11, 3, 15*8,
      ZoneContext::TIME_MODIFIER_W}));
  assertTrue((matches[2].untilDateTime == DateTuple{20, 2, 1, 0,
      ZoneContext::TIME_MODIFIER_W}));
  assertTrue(&kZoneEraAlmostLosAngeles[2] == matches[2].era.zoneEra());
}

test(ExtendedZoneProcessorTest, findMatches_named) {
  YearMonthTuple startYm = {18, 12};
  YearMonthTuple untilYm = {20, 2};
  const uint8_t kMaxMaches = 4;
  ZoneMatch matches[kMaxMaches];
  uint8_t numMatches = ExtendedZoneProcessor::findMatches(
      extended::ZoneInfoBroker(&kZoneTestLos_Angeles), startYm, untilYm,
      matches, kMaxMaches);
  assertEqual(1, numMatches);

  assertTrue((matches[0].startDateTime == DateTuple{18, 12, 1, 0,
      ZoneContext::TIME_MODIFIER_W}));
  assertTrue((matches[0].untilDateTime == DateTuple{20, 2, 1, 0,
      ZoneContext::TIME_MODIFIER_W}));
  assertTrue(&kZoneEraTestLos_Angeles[0] == matches[0].era.zoneEra());
}

test(ExtendedZoneProcessorTest, getTransitionTime) {
  // Nov Sun>=1
  const auto rule = extended::ZoneRuleBroker(&kZoneRulesTestUS[4]);

  // Nov 4 2018
  DateTuple dt = ExtendedZoneProcessor::getTransitionTime(18, rule);
  assertTrue((dt == DateTuple{18, 11, 4, 15*8, ZoneContext::TIME_MODIFIER_W}));

  // Nov 3 2019
  dt = ExtendedZoneProcessor::getTransitionTime(19, rule);
  assertTrue((dt == DateTuple{19, 11, 3, 15*8, ZoneContext::TIME_MODIFIER_W}));
}

test(ExtendedZoneProcessorTest, createTransitionForYear) {
  const ZoneMatch match = {
    {18, 12, 1, 0, ZoneContext::TIME_MODIFIER_W},
    {20, 2, 1, 0, ZoneContext::TIME_MODIFIER_W},
    extended::ZoneEraBroker(&kZoneEraTestLos_Angeles[0])
  };
  // Nov Sun>=1
  const auto rule = extended::ZoneRuleBroker(&kZoneRulesTestUS[4]);
  Transition t;
  ExtendedZoneProcessor::createTransitionForYear(&t, 19, rule, &match);
  assertTrue((t.transitionTime == DateTuple{19, 11, 3, 15*8,
      ZoneContext::TIME_MODIFIER_W}));
}

test(ExtendedZoneProcessorTest, normalizeDateTuple) {
  DateTuple dtp;

  dtp = {0, 1, 1, 0, ZoneContext::TIME_MODIFIER_W};
  ExtendedZoneProcessor::normalizeDateTuple(&dtp);
  assertTrue((dtp == DateTuple{0, 1, 1, 0, ZoneContext::TIME_MODIFIER_W}));

  dtp = {0, 1, 1, 15*95, ZoneContext::TIME_MODIFIER_W}; // 23:45
  ExtendedZoneProcessor::normalizeDateTuple(&dtp);
  assertTrue((dtp == DateTuple{0, 1, 1, 15*95, ZoneContext::TIME_MODIFIER_W}));

  dtp = {0, 1, 1, 15*96, ZoneContext::TIME_MODIFIER_W}; // 24:00
  ExtendedZoneProcessor::normalizeDateTuple(&dtp);
  assertTrue((dtp == DateTuple{0, 1, 2, 0, ZoneContext::TIME_MODIFIER_W}));

  dtp = {0, 1, 1, 15*97, ZoneContext::TIME_MODIFIER_W}; // 24:15
  ExtendedZoneProcessor::normalizeDateTuple(&dtp);
  assertTrue((dtp == DateTuple{0, 1, 2, 15, ZoneContext::TIME_MODIFIER_W}));

  dtp = {0, 1, 1, -15*96, ZoneContext::TIME_MODIFIER_W}; // -24:00
  ExtendedZoneProcessor::normalizeDateTuple(&dtp);
  assertTrue((dtp == DateTuple{-1, 12, 31, 0, ZoneContext::TIME_MODIFIER_W}));

  dtp = {0, 1, 1, -15*97, ZoneContext::TIME_MODIFIER_W}; // -24:15
  ExtendedZoneProcessor::normalizeDateTuple(&dtp);
  assertTrue((dtp == DateTuple{-1, 12, 31, -15, ZoneContext::TIME_MODIFIER_W}));
}

test(ExtendedZoneProcessorTest, expandDateTuple) {
  DateTuple tt;
  DateTuple tts;
  DateTuple ttu;
  int8_t offsetCode = 8;
  int8_t deltaCode = 4;

  tt = {0, 1, 30, 15*12, ZoneContext::TIME_MODIFIER_W}; // 03:00
  ExtendedZoneProcessor::expandDateTuple(&tt, &tts, &ttu,
      offsetCode, deltaCode);
  assertTrue((tt == DateTuple{0, 1, 30, 15*12, ZoneContext::TIME_MODIFIER_W}));
  assertTrue((tts == DateTuple{0, 1, 30, 15*8, ZoneContext::TIME_MODIFIER_S}));
  assertTrue((ttu == DateTuple{0, 1, 30, 0, ZoneContext::TIME_MODIFIER_U}));

  tt = {0, 1, 30, 15*8, ZoneContext::TIME_MODIFIER_S};
  ExtendedZoneProcessor::expandDateTuple(&tt, &tts, &ttu,
      offsetCode, deltaCode);
  assertTrue((tt == DateTuple{0, 1, 30, 15*12, ZoneContext::TIME_MODIFIER_W}));
  assertTrue((tts == DateTuple{0, 1, 30, 15*8, ZoneContext::TIME_MODIFIER_S}));
  assertTrue((ttu == DateTuple{0, 1, 30, 0, ZoneContext::TIME_MODIFIER_U}));

  tt = {0, 1, 30, 0, ZoneContext::TIME_MODIFIER_U};
  ExtendedZoneProcessor::expandDateTuple(&tt, &tts, &ttu,
      offsetCode, deltaCode);
  assertTrue((tt == DateTuple{0, 1, 30, 15*12, ZoneContext::TIME_MODIFIER_W}));
  assertTrue((tts == DateTuple{0, 1, 30, 15*8, ZoneContext::TIME_MODIFIER_S}));
  assertTrue((ttu == DateTuple{0, 1, 30, 0, ZoneContext::TIME_MODIFIER_U}));
}

test(ExtendedZoneProcessorTest, calcInteriorYears) {
  const uint8_t kMaxInteriorYears = 4;
  int8_t interiorYears[kMaxInteriorYears];

  uint8_t num = ExtendedZoneProcessor::calcInteriorYears(
      interiorYears, kMaxInteriorYears, -2, -1, 0, 2);
  assertEqual(0, num);

  num = ExtendedZoneProcessor::calcInteriorYears(
      interiorYears, kMaxInteriorYears, 3, 5, 0, 2);
  assertEqual(0, num);

  num = ExtendedZoneProcessor::calcInteriorYears(
      interiorYears, kMaxInteriorYears, -2, 0, 0, 2);
  assertEqual(1, num);
  assertEqual(0, interiorYears[0]);

  num = ExtendedZoneProcessor::calcInteriorYears(
      interiorYears, kMaxInteriorYears, 2, 4, 0, 2);
  assertEqual(1, num);
  assertEqual(2, interiorYears[0]);

  num = ExtendedZoneProcessor::calcInteriorYears(
      interiorYears, kMaxInteriorYears, 1, 2, 0, 2);
  assertEqual(2, num);
  assertEqual(1, interiorYears[0]);
  assertEqual(2, interiorYears[1]);

  num = ExtendedZoneProcessor::calcInteriorYears(
      interiorYears, kMaxInteriorYears, -1, 3, 0, 2);
  assertEqual(3, num);
  assertEqual(0, interiorYears[0]);
  assertEqual(1, interiorYears[1]);
  assertEqual(2, interiorYears[2]);
}

test(ExtendedZoneProcessorTest, getMostRecentPriorYear) {
  int8_t yearTiny;

  yearTiny = ExtendedZoneProcessor::getMostRecentPriorYear(-2, -1, 0, 2);
  assertEqual(-1, yearTiny);

  yearTiny = ExtendedZoneProcessor::getMostRecentPriorYear(3, 5, 0, 2);
  assertEqual(LocalDate::kInvalidYearTiny, yearTiny);

  yearTiny = ExtendedZoneProcessor::getMostRecentPriorYear(-2, 0, 0, 2);
  assertEqual(-1, yearTiny);

  yearTiny = ExtendedZoneProcessor::getMostRecentPriorYear(2, 4, 0, 2);
  assertEqual(LocalDate::kInvalidYearTiny, yearTiny);

  yearTiny = ExtendedZoneProcessor::getMostRecentPriorYear(1, 2, 0, 2);
  assertEqual(LocalDate::kInvalidYearTiny, yearTiny);

  yearTiny = ExtendedZoneProcessor::getMostRecentPriorYear(-1, 3, 0, 2);
  assertEqual(-1, yearTiny);
}

test(ExtendedZoneProcessorTest, compareTransitionToMatchFuzzy) {
  const DateTuple EMPTY_DATE = { 0, 0, 0, 0, 0};

  const ZoneMatch match = {
    {0, 1, 1, 0, ZoneContext::TIME_MODIFIER_W} /* startDateTime */,
    {1, 1, 1, 0, ZoneContext::TIME_MODIFIER_W} /* untilDateTime */,
    extended::ZoneEraBroker(nullptr)
  };

  Transition transition = {
    &match /*match*/, extended::ZoneRuleBroker(nullptr) /*rule*/,
    {-1, 11, 1, 0, ZoneContext::TIME_MODIFIER_W} /*transitionTime*/,
    EMPTY_DATE, EMPTY_DATE, EMPTY_DATE, 0, {0}, {0}, false, 0, 0
  };
  assertEqual(-1, ExtendedZoneProcessor::compareTransitionToMatchFuzzy(
      &transition, &match));

  transition = {
    &match /*match*/, extended::ZoneRuleBroker(nullptr) /*rule*/,
    {-1, 12, 1, 0, ZoneContext::TIME_MODIFIER_W} /*transitionTime*/,
    EMPTY_DATE, EMPTY_DATE, EMPTY_DATE, 0, {0}, {0}, false, 0, 0
  };
  assertEqual(1, ExtendedZoneProcessor::compareTransitionToMatchFuzzy(
      &transition, &match));

  transition = {
    &match /*match*/, extended::ZoneRuleBroker(nullptr) /*rule*/,
    {0, 1, 1, 0, ZoneContext::TIME_MODIFIER_W} /*transitionTime*/,
    EMPTY_DATE, EMPTY_DATE, EMPTY_DATE, 0, {0}, {0}, false, 0, 0
  };
  assertEqual(1, ExtendedZoneProcessor::compareTransitionToMatchFuzzy(
      &transition, &match));

  transition = {
    &match /*match*/, extended::ZoneRuleBroker(nullptr) /*rule*/,
    {1, 1, 1, 0, ZoneContext::TIME_MODIFIER_W} /*transitionTime*/,
    EMPTY_DATE, EMPTY_DATE, EMPTY_DATE, 0, {0}, {0}, false, 0, 0
  };
  assertEqual(1, ExtendedZoneProcessor::compareTransitionToMatchFuzzy(
      &transition, &match));

  transition = {
    &match /*match*/, extended::ZoneRuleBroker(nullptr) /*rule*/,
    {1, 2, 1, 0, ZoneContext::TIME_MODIFIER_W} /*transitionTime*/,
    EMPTY_DATE, EMPTY_DATE, EMPTY_DATE, 0, {0}, {0}, false, 0, 0
  };
  assertEqual(1, ExtendedZoneProcessor::compareTransitionToMatchFuzzy(
      &transition, &match));

  transition = {
    &match /*match*/, extended::ZoneRuleBroker(nullptr) /*rule*/,
    {1, 3, 1, 0, ZoneContext::TIME_MODIFIER_W} /*transitionTime*/,
    EMPTY_DATE, EMPTY_DATE, EMPTY_DATE, 0, {0}, {0}, false, 0, 0
  };
  assertEqual(2, ExtendedZoneProcessor::compareTransitionToMatchFuzzy(
      &transition, &match));
}


test(ExtendedZoneProcessorTest, compareTransitionToMatch) {
  const DateTuple EMPTY_DATE = { 0, 0, 0, 0, 0};

  const ZoneMatch match = {
    {0, 1, 1, 0, ZoneContext::TIME_MODIFIER_W} /*startDateTime*/,
    {1, 1, 1, 0, ZoneContext::TIME_MODIFIER_W} /*untilDateTime*/,
    extended::ZoneEraBroker(nullptr) /*era*/
  };

  Transition transition = {
    &match /*match*/, extended::ZoneRuleBroker(nullptr) /*rule*/,
    {-1, 12, 31, 0, ZoneContext::TIME_MODIFIER_W} /*transitionTime*/,
    EMPTY_DATE, EMPTY_DATE, EMPTY_DATE, 0, {0}, {0}, false, 0, 0
  };
  assertEqual(-1, ExtendedZoneProcessor::compareTransitionToMatch(
      &transition, &match));

  transition = {
    &match /*match*/, extended::ZoneRuleBroker(nullptr) /*rule*/,
    {0, 1, 1, 0, ZoneContext::TIME_MODIFIER_W} /*transitionTime*/,
    EMPTY_DATE, EMPTY_DATE, EMPTY_DATE, 0, {0}, {0}, false, 0, 0
  };
  assertEqual(0, ExtendedZoneProcessor::compareTransitionToMatch(
      &transition, &match));

  transition = {
    &match /*match*/, extended::ZoneRuleBroker(nullptr) /*rule*/,
    {0, 1, 2, 0, ZoneContext::TIME_MODIFIER_W} /*transitionTime*/,
    EMPTY_DATE, EMPTY_DATE, EMPTY_DATE, 0, {0}, {0}, false, 0, 0
  };
  assertEqual(1, ExtendedZoneProcessor::compareTransitionToMatch(
      &transition, &match));

  transition = {
    &match /*match*/, extended::ZoneRuleBroker(nullptr) /*rule*/,
    {1, 1, 2, 0, ZoneContext::TIME_MODIFIER_W} /*transitionTime*/,
    EMPTY_DATE, EMPTY_DATE, EMPTY_DATE, 0, {0}, {0}, false, 0, 0
  };
  assertEqual(2, ExtendedZoneProcessor::compareTransitionToMatch(
      &transition, &match));
}

test(ExtendedZoneProcessorTest, processActiveTransition) {
  const DateTuple EMPTY_DATE = { 0, 0, 0, 0, 0};

  Transition* prior = nullptr;
  const ZoneMatch match = {
    {0, 1, 1, 0, ZoneContext::TIME_MODIFIER_W} /*startDateTime*/,
    {1, 1, 1, 0, ZoneContext::TIME_MODIFIER_W} /*untilDateTime*/,
    extended::ZoneEraBroker(nullptr) /*era*/
  };

  // This transition occurs before the match, so prior should be filled.
  Transition transition0 = {
    &match /*match*/, extended::ZoneRuleBroker(nullptr) /*rule*/,
    {-1, 12, 31, 0, ZoneContext::TIME_MODIFIER_W} /*transitionTime*/,
    EMPTY_DATE, EMPTY_DATE, EMPTY_DATE, 0, {0}, {0}, false, 0, 0
  };
  ExtendedZoneProcessor::processActiveTransition(&match, &transition0, &prior);
  assertTrue(transition0.active);
  assertTrue(prior == &transition0);

  // This occurs at exactly match.startDateTime, so should replace
  Transition transition1 = {
    &match /*match*/, extended::ZoneRuleBroker(nullptr) /*rule*/,
    {0, 1, 1, 0, ZoneContext::TIME_MODIFIER_W} /*transitionTime*/,
    EMPTY_DATE, EMPTY_DATE, EMPTY_DATE, 0, {0}, {0}, false, 0, 0
  };
  ExtendedZoneProcessor::processActiveTransition(&match, &transition1, &prior);
  assertTrue(transition1.active);
  assertTrue(prior == &transition1);

  // An interior transition. Prior should not change.
  Transition transition2 = {
    &match /*match*/, extended::ZoneRuleBroker(nullptr) /*rule*/,
    {0, 1, 2, 0, ZoneContext::TIME_MODIFIER_W} /*transitionTime*/,
    EMPTY_DATE, EMPTY_DATE, EMPTY_DATE, 0, {0}, {0}, false, 0, 0
  };
  ExtendedZoneProcessor::processActiveTransition(&match, &transition2, &prior);
  assertTrue(transition2.active);
  assertTrue(prior == &transition1);

  // Occurs after match.untilDateTime, so should be rejected.
  Transition transition3 = {
    &match /*match*/, extended::ZoneRuleBroker(nullptr) /*rule*/,
    {1, 1, 2, 0, ZoneContext::TIME_MODIFIER_W} /*transitionTime*/,
    EMPTY_DATE, EMPTY_DATE, EMPTY_DATE, 0, {0}, {0}, false, 0, 0
  };
  assertFalse(transition3.active);
  assertTrue(prior == &transition1);
}

test(ExtendedZoneProcessorTest, findCandidateTransitions) {
  const ZoneMatch match = {
    {18, 12, 1, 0, ZoneContext::TIME_MODIFIER_W},
    {20, 2, 1, 0, ZoneContext::TIME_MODIFIER_W},
    extended::ZoneEraBroker(&kZoneEraTestLos_Angeles[0])
  };

  // Reserve storage for the Transitions
  const uint8_t kMaxStorage = 8;
  TransitionStorage<kMaxStorage> storage;
  storage.init();

  // Verify compareTransitionToMatchFuzzy() elminates various transitions
  // to get down to 5:
  //    * 2018 Mar Sun>=8 (11)
  //    * 2019 Nov Sun>=1 (4)
  //    * 2019 Mar Sun>=8 (10)
  //    * 2019 Nov Sun>=1 (3)
  //    * 2020 Mar Sun>=8 (8)
  storage.resetCandidatePool();
  ExtendedZoneProcessor::findCandidateTransitions(storage, &match);
  assertEqual(5,
      (int) (storage.getCandidatePoolEnd() - storage.getCandidatePoolBegin()));
  Transition** t = storage.getCandidatePoolBegin();
  assertTrue(((*t++)->transitionTime == DateTuple{18, 3, 11, 15*8,
      ZoneContext::TIME_MODIFIER_W}));
  assertTrue(((*t++)->transitionTime == DateTuple{18, 11, 4, 15*8,
      ZoneContext::TIME_MODIFIER_W}));
  assertTrue(((*t++)->transitionTime == DateTuple{19, 3, 10, 15*8,
      ZoneContext::TIME_MODIFIER_W}));
  assertTrue(((*t++)->transitionTime == DateTuple{19, 11, 3, 15*8,
      ZoneContext::TIME_MODIFIER_W}));
  assertTrue(((*t++)->transitionTime == DateTuple{20, 3, 8, 15*8,
      ZoneContext::TIME_MODIFIER_W}));
}

test(ExtendedZoneProcessorTest, findTransitionsFromNamedMatch) {
  const ZoneMatch match = {
    {18, 12, 1, 0, ZoneContext::TIME_MODIFIER_W},
    {20, 2, 1, 0, ZoneContext::TIME_MODIFIER_W},
    extended::ZoneEraBroker(&kZoneEraTestLos_Angeles[0])
  };

  // Reserve storage for the Transitions
  const uint8_t kMaxStorage = 8;
  TransitionStorage<kMaxStorage> storage;
  storage.init();

  ExtendedZoneProcessor::findTransitionsFromNamedMatch(storage, &match);
  assertEqual(3,
      (int) (storage.getActivePoolEnd() - storage.getActivePoolBegin()));
  Transition** t = storage.getActivePoolBegin();
  assertTrue(((*t++)->transitionTime == DateTuple{18, 12, 1, 0,
      ZoneContext::TIME_MODIFIER_W}));
  assertTrue(((*t++)->transitionTime == DateTuple{19, 3, 10, 15*8,
      ZoneContext::TIME_MODIFIER_W}));
  assertTrue(((*t++)->transitionTime == DateTuple{19, 11, 3, 15*8,
      ZoneContext::TIME_MODIFIER_W}));
}

test(ExtendedZoneProcessorTest, fixTransitionTimes_generateStartUntilTimes) {
  // Create 3 matches for the AlmostLosAngeles test zone.
  YearMonthTuple startYm = {18, 12};
  YearMonthTuple untilYm = {20, 2};
  const uint8_t kMaxMaches = 4;
  ZoneMatch matches[kMaxMaches];
  uint8_t numMatches = ExtendedZoneProcessor::findMatches(
      extended::ZoneInfoBroker(&kZoneAlmostLosAngeles), startYm, untilYm,
      matches, kMaxMaches);
  assertEqual(3, numMatches);

  TransitionStorage<4> storage;
  storage.init();

  // Create 3 Transitions corresponding to the matches.
  // Implements ExtendedZoneProcessor::findTransitionsFromSimpleMatch().
  Transition* transition1 = storage.getFreeAgent();
  ExtendedZoneProcessor::createTransitionForYear(
      transition1, 0 /*not used*/, extended::ZoneRuleBroker(nullptr) /*rule*/,
      &matches[0]);
  transition1->active = true;
  storage.addFreeAgentToCandidatePool();

  Transition* transition2 = storage.getFreeAgent();
  ExtendedZoneProcessor::createTransitionForYear(
      transition2, 0 /*not used*/, extended::ZoneRuleBroker(nullptr) /*rule*/,
      &matches[1]);
  transition2->active = true;
  storage.addFreeAgentToCandidatePool();

  Transition* transition3 = storage.getFreeAgent();
  ExtendedZoneProcessor::createTransitionForYear(
      transition3, 0 /*not used*/, extended::ZoneRuleBroker(nullptr) /*rule*/,
      &matches[2]);
  transition3->active = true;
  storage.addFreeAgentToCandidatePool();

  // Move actives to Active pool.
  storage.addActiveCandidatesToActivePool();
  Transition** begin = storage.getActivePoolBegin();
  Transition** end = storage.getActivePoolEnd();
  assertEqual(3, (int) (end - begin));
  assertTrue(begin[0] == transition1);
  assertTrue(begin[1] == transition2);
  assertTrue(begin[2] == transition3);

  // Fix the transition times, expanding to 's' and 'u'
  ExtendedZoneProcessor::fixTransitionTimes(begin, end);

  // Verify. The first Transition is extended to -infinity.
  assertTrue((transition1->transitionTime == DateTuple{18, 12, 1, 0,
      ZoneContext::TIME_MODIFIER_W}));
  assertTrue((transition1->transitionTimeS == DateTuple{18, 12, 1, 0,
      ZoneContext::TIME_MODIFIER_S}));
  assertTrue((transition1->transitionTimeU == DateTuple{18, 12, 1, 15*32,
      ZoneContext::TIME_MODIFIER_U}));

  // Second transition uses the UTC offset of the first.
  assertTrue((transition2->transitionTime == DateTuple{19, 3, 10, 15*8,
      ZoneContext::TIME_MODIFIER_W}));
  assertTrue((transition2->transitionTimeS == DateTuple{19, 3, 10, 15*8,
      ZoneContext::TIME_MODIFIER_S}));
  assertTrue((transition2->transitionTimeU == DateTuple{19, 3, 10, 15*40,
      ZoneContext::TIME_MODIFIER_U}));

  // Third transition uses the UTC offset of the second.
  assertTrue((transition3->transitionTime == DateTuple{19, 11, 3, 15*8,
      ZoneContext::TIME_MODIFIER_W}));
  assertTrue((transition3->transitionTimeS == DateTuple{19, 11, 3, 15*4,
      ZoneContext::TIME_MODIFIER_S}));
  assertTrue((transition3->transitionTimeU == DateTuple{19, 11, 3, 15*36,
      ZoneContext::TIME_MODIFIER_U}));

  // Generate the startDateTime and untilDateTime of the transitions.
  ExtendedZoneProcessor::generateStartUntilTimes(begin, end);

  // Verify. The first transition startTime should be the same as its
  // transitionTime.
  assertTrue((transition1->transitionTime == DateTuple{18, 12, 1, 0,
      ZoneContext::TIME_MODIFIER_W}));
  assertTrue((transition1->startDateTime == DateTuple{18, 12, 1, 0,
      ZoneContext::TIME_MODIFIER_W}));
  assertTrue((transition1->untilDateTime == DateTuple{19, 3, 10, 15*8,
      ZoneContext::TIME_MODIFIER_W}));
  acetime_t epochSecs = OffsetDateTime::forComponents(
      2018, 12, 1, 0, 0, 0, TimeOffset::forHours(-8)).toEpochSeconds();
  assertEqual(epochSecs, transition1->startEpochSeconds);

  // Second transition startTime is shifted forward one hour into PDT.
  assertTrue((transition2->transitionTime == DateTuple{19, 3, 10, 15*8,
      ZoneContext::TIME_MODIFIER_W}));
  assertTrue((transition2->startDateTime == DateTuple{19, 3, 10, 15*12,
      ZoneContext::TIME_MODIFIER_W}));
  assertTrue((transition2->untilDateTime == DateTuple{19, 11, 3, 15*8,
      ZoneContext::TIME_MODIFIER_W}));
  epochSecs = OffsetDateTime::forComponents(
      2019, 3, 10, 3, 0, 0, TimeOffset::forHours(-7)).toEpochSeconds();
  assertEqual(epochSecs, transition2->startEpochSeconds);

  // Third transition startTime is shifted back one hour into PST.
  assertTrue((transition3->transitionTime == DateTuple{19, 11, 3, 15*8,
      ZoneContext::TIME_MODIFIER_W}));
  assertTrue((transition3->startDateTime == DateTuple{19, 11, 3, 15*4,
      ZoneContext::TIME_MODIFIER_W}));
  assertTrue((transition3->untilDateTime == DateTuple{20, 2, 1, 0,
      ZoneContext::TIME_MODIFIER_W}));
  epochSecs = OffsetDateTime::forComponents(
      2019, 11, 3, 1, 0, 0, TimeOffset::forHours(-8)).toEpochSeconds();
  assertEqual(epochSecs, transition3->startEpochSeconds);
}

test(ExtendedZoneProcessorTest, createAbbreviation) {
  const uint8_t kDstSize = 6;
  char dst[kDstSize];

  ExtendedZoneProcessor::createAbbreviation(dst, kDstSize, "SAST", 0, nullptr);
  assertEqual("SAST", dst);

  ExtendedZoneProcessor::createAbbreviation(dst, kDstSize, "P%T", 4, "D");
  assertEqual("PDT", dst);

  ExtendedZoneProcessor::createAbbreviation(dst, kDstSize, "P%T", 0, "S");
  assertEqual("PST", dst);

  ExtendedZoneProcessor::createAbbreviation(dst, kDstSize, "P%T", 0, "");
  assertEqual("PT", dst);

  ExtendedZoneProcessor::createAbbreviation(dst, kDstSize, "GMT/BST", 0, "");
  assertEqual("GMT", dst);

  ExtendedZoneProcessor::createAbbreviation(dst, kDstSize, "GMT/BST", 4, "");
  assertEqual("BST", dst);

  ExtendedZoneProcessor::createAbbreviation(dst, kDstSize, "P%T3456", 4, "DD");
  assertEqual("PDDT3", dst);

  ExtendedZoneProcessor::createAbbreviation(dst, kDstSize, "%", 4, "CAT");
  assertEqual("CAT", dst);

  ExtendedZoneProcessor::createAbbreviation(dst, kDstSize, "%", 0, "WAT");
  assertEqual("WAT", dst);
}

test(ExtendedZoneProcessorTest, calcAbbreviations) {
  // TODO: Implement
}


// --------------------------------------------------------------------------
// Test public methods
// --------------------------------------------------------------------------

test(ExtendedZoneProcessorTest, setZoneInfo) {
  ExtendedZoneProcessor zoneInfo(&zonedbx::kZoneAmerica_Los_Angeles);
  zoneInfo.getUtcOffset(0);
  assertTrue(zoneInfo.mIsFilled);

  zoneInfo.setZoneInfo(&zonedbx::kZoneAustralia_Darwin);
  assertFalse(zoneInfo.mIsFilled);
  zoneInfo.getUtcOffset(0);
  assertTrue(zoneInfo.mIsFilled);

  // Check that the cache remains valid if the zoneInfo does not change
  zoneInfo.setZoneInfo(&zonedbx::kZoneAustralia_Darwin);
  assertTrue(zoneInfo.mIsFilled);
}

// --------------------------------------------------------------------------

void setup() {
#if defined(ARDUINO)
  delay(1000); // wait for stability to prevent garbage on SERIAL_PORT_MONITOR
#endif
  SERIAL_PORT_MONITOR.begin(115200);
  while(!SERIAL_PORT_MONITOR); // for the Arduino Leonardo/Micro only

#if 0
  TestRunner::exclude("*");
  TestRunner::include("ExtendedZoneProcessorTest",
      "findTransitionsFromNamedMatch");
#endif
}

void loop() {
  TestRunner::run();
}
