#line 2 "ExtendedZoneProcessorTest.ino"

// Tests for the private/protected implementation functions of
// ExtendedZoneProcessor. The public ones have been moved to
// ExtendedZoneProcessorMoreTest because they became too big for the ~30kB
// limit of a Sparkfun Pro Micro.

#include <AUnit.h>
#include <AceCommon.h> // PrintStr<>
#include <AceTime.h>

using ace_common::PrintStr;
using namespace ace_time;
using ace_time::extended::DateTuple;
using ace_time::extended::TransitionStorageTemplate;
using ace_time::extended::YearMonthTuple;
using ace_time::extended::ZoneEra;
using ace_time::extended::ZoneEraBroker;
using ace_time::extended::ZoneInfo;
using ace_time::extended::ZoneInfoBroker;
using ace_time::extended::ZonePolicy;
using ace_time::extended::ZonePolicyBroker;
using ace_time::extended::ZoneRule;
using ace_time::extended::ZoneRuleBroker;
using ace_time::extended::normalizeDateTuple;
using ace_time::extended::subtractDateTuple;
using ace_time::internal::ZoneContext;

//---------------------------------------------------------------------------
// A simplified version of America/Los_Angeles, using only simple ZoneEras
// (i.e. no references to a ZonePolicy). Valid only for 2018.
//---------------------------------------------------------------------------

static const ZoneContext kZoneContext = {
  2000 /*startYear*/,
  2020 /*untilYear*/,
  "testing" /*tzVersion*/,
  0 /*numFragments*/,
  nullptr /*fragments*/,
};

// Create simplified ZoneEras which approximate America/Los_Angeles
static const ZoneEra kZoneEraAlmostLosAngeles[] ACE_TIME_PROGMEM = {
  {
    nullptr,
    "PST" /*format*/,
    -32 /*offsetCode*/,
    0 + 4 /*deltaCode*/,
    2019 /*untilYear*/,
    3 /*untilMonth*/,
    10 /*untilDay*/,
    2*4 /*untilTimeCode*/,
    ZoneContext::kSuffixW /*untilTimeModifier*/
  },
  {
    nullptr,
    "PDT" /*format*/,
    -32 /*offsetCode*/,
    4 + 4 /*deltaCode*/,
    2019 /*untilYear*/,
    11 /*untilMonth*/,
    3 /*untilDay*/,
    2*4 /*untilTimeCode*/,
    ZoneContext::kSuffixW /*untilTimeModifier*/
  },
  {
    nullptr,
    "PST" /*format*/,
    -32 /*offsetCode*/,
    0 + 4 /*deltaCode*/,
    2020 /*untilYear*/,
    3 /*untilMonth*/,
    8 /*untilDay*/,
    2*4 /*untilTimeCode*/,
    ZoneContext::kSuffixW /*untilTimeModifier*/
  },
};

static const ZoneInfo kZoneAlmostLosAngeles ACE_TIME_PROGMEM = {
  "Almost_Los_Angeles" /*name*/,
  0x70166020 /*zoneId*/,
  &::kZoneContext /*zoneContext*/,
  3 /*numEras*/,
  kZoneEraAlmostLosAngeles /*eras*/,
};

//---------------------------------------------------------------------------
// A real ZoneInfo for America/Los_Angeles. Taken from zonedbx/zone_infos.cpp.
//---------------------------------------------------------------------------

static const ZoneRule kZoneRulesTestUS[] ACE_TIME_PROGMEM = {
  // Rule    US    1967    2006    -    Oct    lastSun    2:00    0    S
  {
    1967 /*fromYear*/,
    2006 /*toYear*/,
    10 /*inMonth*/,
    7 /*onDayOfWeek*/,
    0 /*onDayOfMonth*/,
    8 /*atTimeCode*/,
    ZoneContext::kSuffixW /*atTimeModifier*/,
    0 + 4 /*deltaCode*/,
    'S' /*letter*/,
  },
  // Rule    US    1976    1986    -    Apr    lastSun    2:00    1:00    D
  {
    1976 /*fromYear*/,
    1986 /*toYear*/,
    4 /*inMonth*/,
    7 /*onDayOfWeek*/,
    0 /*onDayOfMonth*/,
    8 /*atTimeCode*/,
    ZoneContext::kSuffixW /*atTimeModifier*/,
    4 + 4 /*deltaCode*/,
    'D' /*letter*/,
  },
  // Rule    US    1987    2006    -    Apr    Sun>=1    2:00    1:00    D
  {
    1987 /*fromYear*/,
    2006 /*toYear*/,
    4 /*inMonth*/,
    7 /*onDayOfWeek*/,
    1 /*onDayOfMonth*/,
    8 /*atTimeCode*/,
    ZoneContext::kSuffixW /*atTimeModifier*/,
    4 + 4 /*deltaCode*/,
    'D' /*letter*/,
  },
  // Rule    US    2007    max    -    Mar    Sun>=8    2:00    1:00    D
  {
    2007 /*fromYear*/,
    9999 /*toYear*/,
    3 /*inMonth*/,
    7 /*onDayOfWeek*/,
    8 /*onDayOfMonth*/,
    8 /*atTimeCode*/,
    ZoneContext::kSuffixW /*atTimeModifier*/,
    4 + 4 /*deltaCode*/,
    'D' /*letter*/,
  },
  // Rule    US    2007    max    -    Nov    Sun>=1    2:00    0    S
  {
    2007 /*fromYear*/,
    9999 /*toYear*/,
    11 /*inMonth*/,
    7 /*onDayOfWeek*/,
    1 /*onDayOfMonth*/,
    8 /*atTimeCode*/,
    ZoneContext::kSuffixW /*atTimeModifier*/,
    0 + 4 /*deltaCode*/,
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
    0 + 4 /*deltaCode*/,
    10000 /*untilYear*/,
    1 /*untilMonth*/,
    1 /*untilDay*/,
    0 /*untilTimeCode*/,
    ZoneContext::kSuffixW /*untilTimeModifier*/,
  },

};

static const ZoneInfo kZoneTestLos_Angeles ACE_TIME_PROGMEM = {
  "America/Los_Angeles" /*name*/,
  0xb7f7e8f2 /*zoneId*/,
  &::kZoneContext /*zoneContext*/,
  1 /*numEras*/,
  kZoneEraTestLos_Angeles /*eras*/,
};

//---------------------------------------------------------------------------
// Step 1
//---------------------------------------------------------------------------

static const ZoneEra era ACE_TIME_PROGMEM =
    {nullptr, "", 0, 0, 2000, 1, 2, 12, ZoneContext::kSuffixW};

test(ExtendedZoneProcessorTest, compareEraToYearMonth) {
  assertEqual(1, ExtendedZoneProcessor::compareEraToYearMonth(
      ZoneEraBroker(&era), 2000, 1));
  assertEqual(1, ExtendedZoneProcessor::compareEraToYearMonth(
      ZoneEraBroker(&era), 2000, 1));
  assertEqual(-1, ExtendedZoneProcessor::compareEraToYearMonth(
      ZoneEraBroker(&era), 2000, 2));
  assertEqual(-1, ExtendedZoneProcessor::compareEraToYearMonth(
      ZoneEraBroker(&era), 2000, 3));
}

static const ZoneEra era2 ACE_TIME_PROGMEM =
    {nullptr, "", 0, 0, 2000, 1, 0, 0, ZoneContext::kSuffixW};

test(ExtendedZoneProcessorTest, compareEraToYearMonth2) {
  assertEqual(0, ExtendedZoneProcessor::compareEraToYearMonth(
      ZoneEraBroker(&era2), 2000, 1));
}

test(ExtendedZoneProcessorTest, createMatchingEra) {
  // 14-month interval, from 2000-12 until 2002-02
  YearMonthTuple startYm = {2000, 12};
  YearMonthTuple untilYm = {2002, 2};

  // UNTIL = 2000-12-02 3:00
  const ZoneEra era1 ACE_TIME_PROGMEM =
      {nullptr, "", 0, 0, 2000 /*y*/, 12/*m*/, 2/*d*/, 3*(60/15),
      ZoneContext::kSuffixW};

  // UNTIL = 2001-02-03 4:00
  const ZoneEra era2 ACE_TIME_PROGMEM =
      {nullptr, "", 0, 0, 2001/*y*/, 2/*m*/, 3/*d*/, 4*(60/15),
      ZoneContext::kSuffixW};

  // UNTIL = 2002-10-11 4:00
  const ZoneEra era3 ACE_TIME_PROGMEM =
      {nullptr, "", 0, 0, 2002/*y*/, 10/*m*/, 11/*d*/, 4*(60/15),
      ZoneContext::kSuffixW};

  // No previous matching era, so startDateTime is set to startYm.
  ExtendedZoneProcessor::MatchingEra match1 =
      ExtendedZoneProcessor::createMatchingEra(
          nullptr /*prevMatch*/,
          ZoneEraBroker(&era1) /*era*/,
          startYm,
          untilYm);
  assertTrue((match1.startDateTime == DateTuple{2000, 12, 1, 60*0,
      ZoneContext::kSuffixW}));
  assertTrue((match1.untilDateTime == DateTuple{2000, 12, 2, 60*3,
      ZoneContext::kSuffixW}));
  assertTrue(match1.era.equals(ZoneEraBroker(&era1)));

  // startDateTime is set to the prevMatch.untilDateTime.
  // untilDateTime is < untilYm, so is retained.
  ExtendedZoneProcessor::MatchingEra match2 =
      ExtendedZoneProcessor::createMatchingEra(
          &match1,
          ZoneEraBroker(&era2) /*era*/,
          startYm,
          untilYm);
  assertTrue((match2.startDateTime == DateTuple{2000, 12, 2, 60*3,
      ZoneContext::kSuffixW}));
  assertTrue((match2.untilDateTime == DateTuple{2001, 2, 3, 60*4,
      ZoneContext::kSuffixW}));
  assertTrue(match2.era.equals(ZoneEraBroker(&era2)));

  // startDateTime is set to the prevMatch.untilDateTime.
  // untilDateTime is > untilYm so truncated to untilYm.
  ExtendedZoneProcessor::MatchingEra match3 =
      ExtendedZoneProcessor::createMatchingEra(
          &match2,
          ZoneEraBroker(&era3) /*era*/,
          startYm,
          untilYm);
  assertTrue((match3.startDateTime == DateTuple{2001, 2, 3, 60*4,
      ZoneContext::kSuffixW}));
  assertTrue((match3.untilDateTime == DateTuple{2002, 2, 1, 60*0,
      ZoneContext::kSuffixW}));
  assertTrue(match3.era.equals(ZoneEraBroker(&era3)));
}

test(ExtendedZoneProcessorTest, findMatches_simple) {
  YearMonthTuple startYm = {2018, 12};
  YearMonthTuple untilYm = {2020, 2};
  const uint8_t kMaxMatches = 4;
  ExtendedZoneProcessor::MatchingEra matches[kMaxMatches];
  uint8_t numMatches = ExtendedZoneProcessor::findMatches(
      ZoneInfoBroker(&kZoneAlmostLosAngeles), startYm, untilYm,
      matches, kMaxMatches);
  assertEqual(3, numMatches);

  assertTrue((matches[0].startDateTime == DateTuple{2018, 12, 1, 0,
      ZoneContext::kSuffixW}));
  assertTrue((matches[0].untilDateTime == DateTuple{2019, 3, 10, 15*8,
      ZoneContext::kSuffixW}));
  assertTrue(matches[0].era.equals(
      ZoneEraBroker(&kZoneEraAlmostLosAngeles[0])));

  assertTrue((matches[1].startDateTime == DateTuple{2019, 3, 10, 15*8,
      ZoneContext::kSuffixW}));
  assertTrue((matches[1].untilDateTime == DateTuple{2019, 11, 3, 15*8,
      ZoneContext::kSuffixW}));
  assertTrue(matches[1].era.equals(
      ZoneEraBroker(&kZoneEraAlmostLosAngeles[1])));

  assertTrue((matches[2].startDateTime == DateTuple{2019, 11, 3, 15*8,
      ZoneContext::kSuffixW}));
  assertTrue((matches[2].untilDateTime == DateTuple{2020, 2, 1, 0,
      ZoneContext::kSuffixW}));
  assertTrue(matches[2].era.equals(
      ZoneEraBroker(&kZoneEraAlmostLosAngeles[2])));
}

test(ExtendedZoneProcessorTest, findMatches_named) {
  YearMonthTuple startYm = {2018, 12};
  YearMonthTuple untilYm = {2020, 2};
  const uint8_t kMaxMatches = 4;
  ExtendedZoneProcessor::MatchingEra matches[kMaxMatches];
  uint8_t numMatches = ExtendedZoneProcessor::findMatches(
      ZoneInfoBroker(&kZoneTestLos_Angeles), startYm, untilYm,
      matches, kMaxMatches);
  assertEqual(1, numMatches);

  assertTrue((matches[0].startDateTime == DateTuple{2018, 12, 1, 0,
      ZoneContext::kSuffixW}));
  assertTrue((matches[0].untilDateTime == DateTuple{2020, 2, 1, 0,
      ZoneContext::kSuffixW}));
  assertTrue(matches[0].era.equals(
      ZoneEraBroker(&kZoneEraTestLos_Angeles[0])));
}

//---------------------------------------------------------------------------
// Step 2A
//---------------------------------------------------------------------------

test(ExtendedZoneProcessorTest, getTransitionTime) {
  // Nov Sun>=1
  const auto rule = ZoneRuleBroker(&kZoneRulesTestUS[4]);

  // Nov 4 2018
  DateTuple dt = ExtendedZoneProcessor::getTransitionTime(2018, rule);
  assertTrue((dt == DateTuple{2018, 11, 4, 15*8, ZoneContext::kSuffixW}));

  // Nov 3 2019
  dt = ExtendedZoneProcessor::getTransitionTime(2019, rule);
  assertTrue((dt == DateTuple{2019, 11, 3, 15*8, ZoneContext::kSuffixW}));
}

test(ExtendedZoneProcessorTest, createTransitionForYear) {
  const ExtendedZoneProcessor::MatchingEra match = {
    {2018, 12, 1, 0, ZoneContext::kSuffixW},
    {2020, 2, 1, 0, ZoneContext::kSuffixW},
    ZoneEraBroker(&kZoneEraTestLos_Angeles[0]),
    nullptr /*prevMatch*/,
    0 /*lastOffsetMinutes*/,
    0 /*lastDeltaMinutes*/
  };
  // Nov Sun>=1
  const auto rule = ZoneRuleBroker(&kZoneRulesTestUS[4]);
  ExtendedZoneProcessor::Transition t;
  ExtendedZoneProcessor::createTransitionForYear(&t, 2019, rule, &match);
  assertTrue((t.transitionTime == DateTuple{2019, 11, 3, 15*8,
      ZoneContext::kSuffixW}));
}

//---------------------------------------------------------------------------
// Step 2B: Pass 1
//---------------------------------------------------------------------------

test(ExtendedZoneProcessorTest, calcInteriorYears) {
  const uint8_t kMaxInteriorYears = 4;
  int16_t interiorYears[kMaxInteriorYears];

  uint8_t num = ExtendedZoneProcessor::calcInteriorYears(
      interiorYears, kMaxInteriorYears, 1998, 1999, 2000, 2002);
  assertEqual(0, num);

  num = ExtendedZoneProcessor::calcInteriorYears(
      interiorYears, kMaxInteriorYears, 2003, 2005, 2000, 2002);
  assertEqual(0, num);

  num = ExtendedZoneProcessor::calcInteriorYears(
      interiorYears, kMaxInteriorYears, 1998, 2000, 2000, 2002);
  assertEqual(1, num);
  assertEqual(2000, interiorYears[0]);

  num = ExtendedZoneProcessor::calcInteriorYears(
      interiorYears, kMaxInteriorYears, 2002, 2004, 2000, 2002);
  assertEqual(1, num);
  assertEqual(2002, interiorYears[0]);

  num = ExtendedZoneProcessor::calcInteriorYears(
      interiorYears, kMaxInteriorYears, 2001, 2002, 2000, 2002);
  assertEqual(2, num);
  assertEqual(2001, interiorYears[0]);
  assertEqual(2002, interiorYears[1]);

  num = ExtendedZoneProcessor::calcInteriorYears(
      interiorYears, kMaxInteriorYears, 1999, 2003, 2000, 2002);
  assertEqual(3, num);
  assertEqual(2000, interiorYears[0]);
  assertEqual(2001, interiorYears[1]);
  assertEqual(2002, interiorYears[2]);
}

test(ExtendedZoneProcessorTest, getMostRecentPriorYear) {
  int16_t year;

  year = ExtendedZoneProcessor::getMostRecentPriorYear(1998, 1999, 2000, 2002);
  assertEqual(1999, year);

  year = ExtendedZoneProcessor::getMostRecentPriorYear(2003, 2005, 2000, 2002);
  assertEqual(LocalDate::kInvalidYear, year);

  year = ExtendedZoneProcessor::getMostRecentPriorYear(1998, 2000, 2000, 2002);
  assertEqual(1999, year);

  year = ExtendedZoneProcessor::getMostRecentPriorYear(2002, 2004, 2000, 2002);
  assertEqual(LocalDate::kInvalidYear, year);

  year = ExtendedZoneProcessor::getMostRecentPriorYear(2001, 2002, 2000, 2002);
  assertEqual(LocalDate::kInvalidYear, year);

  year = ExtendedZoneProcessor::getMostRecentPriorYear(199, 2003, 2000, 2002);
  assertEqual(1999, year);
}

test(ExtendedZoneProcessorTest, compareDateTupleFuzzy) {
  using ace_time::extended::MatchStatus;
  using ace_time::extended::DateTuple;

  assertEqual(
    (uint8_t) MatchStatus::kPrior,
    (uint8_t) ExtendedZoneProcessor::compareDateTupleFuzzy(
      DateTuple{2000, 10, 1, 1, 0},
      DateTuple{2000, 12, 1, 1, 0},
      DateTuple{2002, 2, 1, 1, 0}));

  assertEqual(
    (uint8_t) MatchStatus::kWithinMatch,
    (uint8_t) ExtendedZoneProcessor::compareDateTupleFuzzy(
      DateTuple{2000, 11, 1, 1, 0},
      DateTuple{2000, 12, 1, 1, 0},
      DateTuple{2002, 2, 1, 1, 0}));

  assertEqual(
    (uint8_t) MatchStatus::kWithinMatch,
    (uint8_t) ExtendedZoneProcessor::compareDateTupleFuzzy(
      DateTuple{2000, 12, 1, 1, 0},
      DateTuple{2000, 12, 1, 1, 0},
      DateTuple{2002, 2, 1, 1, 0}));

  assertEqual(
    (uint8_t) MatchStatus::kWithinMatch,
    (uint8_t) ExtendedZoneProcessor::compareDateTupleFuzzy(
      DateTuple{2002, 2, 1, 1, 0},
      DateTuple{2000, 12, 1, 1, 0},
      DateTuple{2002, 2, 1, 1, 0}));

  assertEqual(
    (uint8_t) MatchStatus::kWithinMatch,
    (uint8_t) ExtendedZoneProcessor::compareDateTupleFuzzy(
      DateTuple{2002, 3, 1, 1, 0},
      DateTuple{2000, 12, 1, 1, 0},
      DateTuple{2002, 2, 1, 1, 0}));

  assertEqual(
    (uint8_t) MatchStatus::kFarFuture,
    (uint8_t) ExtendedZoneProcessor::compareDateTupleFuzzy(
      DateTuple{2002, 4, 1, 1, 0},
      DateTuple{2000, 12, 1, 1, 0},
      DateTuple{2002, 2, 1, 1, 0}));

  // Verify dates whose delta months is greater than 32767. In
  // other words, delta years is greater than 2730.
  assertEqual(
    (uint8_t) MatchStatus::kFarFuture,
    (uint8_t) ExtendedZoneProcessor::compareDateTupleFuzzy(
      DateTuple{5000, 4, 1, 1, 0},
      DateTuple{2000, 12, 1, 1, 0},
      DateTuple{2002, 2, 1, 1, 0}));
  assertEqual(
    (uint8_t) MatchStatus::kPrior,
    (uint8_t) ExtendedZoneProcessor::compareDateTupleFuzzy(
      DateTuple{1000, 4, 1, 1, 0},
      DateTuple{4000, 12, 1, 1, 0},
      DateTuple{4002, 2, 1, 1, 0}));
}

test(ExtendedZoneProcessorTest, compareTransitionToMatchFuzzy) {
  using ace_time::extended::MatchStatus;

  const DateTuple EMPTY_DATE = {0, 0, 0, 0, 0};

  const ExtendedZoneProcessor::MatchingEra match = {
    {2000, 1, 1, 0, ZoneContext::kSuffixW} /* startDateTime */,
    {2001, 1, 1, 0, ZoneContext::kSuffixW} /* untilDateTime */,
    ZoneEraBroker(nullptr),
    nullptr /*prevMatch*/,
    0 /*lastOffsetMinutes*/,
    0 /*lastDeltaMinutes*/
  };

  ExtendedZoneProcessor::Transition transition = {
    &match /*match*/, ZoneRuleBroker(nullptr) /*rule*/,
    {1999, 11, 1, 0, ZoneContext::kSuffixW} /*transitionTime*/,
    EMPTY_DATE, EMPTY_DATE,
  #if ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG
    EMPTY_DATE,
  #endif
    0, 0, 0, {0}, {0}, false
  };
  assertEqual(
      (uint8_t) MatchStatus::kPrior,
      (uint8_t) ExtendedZoneProcessor::compareTransitionToMatchFuzzy(
          &transition, &match));

  transition = {
    &match /*match*/, ZoneRuleBroker(nullptr) /*rule*/,
    {1999, 12, 1, 0, ZoneContext::kSuffixW} /*transitionTime*/,
    EMPTY_DATE, EMPTY_DATE,
  #if ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG
    EMPTY_DATE,
  #endif
    0, 0, 0, {0}, {0}, false
  };
  assertEqual(
      (uint8_t) MatchStatus::kWithinMatch,
      (uint8_t) ExtendedZoneProcessor::compareTransitionToMatchFuzzy(
          &transition, &match));

  transition = {
    &match /*match*/, ZoneRuleBroker(nullptr) /*rule*/,
    {2000, 1, 1, 0, ZoneContext::kSuffixW} /*transitionTime*/,
    EMPTY_DATE, EMPTY_DATE,
  #if ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG
    EMPTY_DATE,
  #endif
    0, 0, 0, {0}, {0}, false
  };
  assertEqual(
      (uint8_t) MatchStatus::kWithinMatch,
      (uint8_t) ExtendedZoneProcessor::compareTransitionToMatchFuzzy(
          &transition, &match));

  transition = {
    &match /*match*/, ZoneRuleBroker(nullptr) /*rule*/,
    {2001, 1, 1, 0, ZoneContext::kSuffixW} /*transitionTime*/,
    EMPTY_DATE, EMPTY_DATE,
  #if ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG
    EMPTY_DATE,
  #endif
    0, 0, 0, {0}, {0}, false
  };
  assertEqual(
      (uint8_t) MatchStatus::kWithinMatch,
      (uint8_t) ExtendedZoneProcessor::compareTransitionToMatchFuzzy(
          &transition, &match));

  transition = {
    &match /*match*/, ZoneRuleBroker(nullptr) /*rule*/,
    {2001, 2, 1, 0, ZoneContext::kSuffixW} /*transitionTime*/,
    EMPTY_DATE, EMPTY_DATE,
  #if ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG
    EMPTY_DATE,
  #endif
    0, 0, 0, {0}, {0}, false
  };
  assertEqual(
      (uint8_t) MatchStatus::kWithinMatch,
      (uint8_t) ExtendedZoneProcessor::compareTransitionToMatchFuzzy(
          &transition, &match));

  transition = {
    &match /*match*/, ZoneRuleBroker(nullptr) /*rule*/,
    {2001, 3, 1, 0, ZoneContext::kSuffixW} /*transitionTime*/,
    EMPTY_DATE, EMPTY_DATE,
  #if ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG
    EMPTY_DATE,
  #endif
    0, 0, 0, {0}, {0}, false
  };
  assertEqual(
      (uint8_t) MatchStatus::kFarFuture,
      (uint8_t) ExtendedZoneProcessor::compareTransitionToMatchFuzzy(
          &transition, &match));
}

test(ExtendedZoneProcessorTest, findCandidateTransitions) {
  const ExtendedZoneProcessor::MatchingEra match = {
    {2018, 12, 1, 0, ZoneContext::kSuffixW},
    {2020, 2, 1, 0, ZoneContext::kSuffixW},
    ZoneEraBroker(&kZoneEraTestLos_Angeles[0]),
    nullptr /*prevMatch*/,
    0 /*lastOffsetMinutes*/,
    0 /*lastDeltaMinutes*/
  };

  // Reserve storage for the Transitions
  ExtendedZoneProcessor::TransitionStorage storage;
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
  ExtendedZoneProcessor::Transition** t = storage.getCandidatePoolBegin();
  assertTrue(((*t++)->transitionTime == DateTuple{2018, 3, 11, 15*8,
      ZoneContext::kSuffixW}));
  assertTrue(((*t++)->transitionTime == DateTuple{2018, 11, 4, 15*8,
      ZoneContext::kSuffixW}));
  assertTrue(((*t++)->transitionTime == DateTuple{2019, 3, 10, 15*8,
      ZoneContext::kSuffixW}));
  assertTrue(((*t++)->transitionTime == DateTuple{2019, 11, 3, 15*8,
      ZoneContext::kSuffixW}));
  assertTrue(((*t++)->transitionTime == DateTuple{2020, 3, 8, 15*8,
      ZoneContext::kSuffixW}));
}

//---------------------------------------------------------------------------
// Step 2B: Pass 2; Step 3
//---------------------------------------------------------------------------

test(ExtendedZoneProcessorTest, expandDateTuple) {
  DateTuple ttw;
  DateTuple tts;
  DateTuple ttu;
  int16_t offsetMinutes = 2*60;
  int16_t deltaMinutes = 1*60;

  DateTuple tt = {2000, 1, 30, 15*16, ZoneContext::kSuffixW}; // 04:00
  ExtendedZoneProcessor::expandDateTuple(
      &tt, offsetMinutes, deltaMinutes,
      &ttw, &tts, &ttu);
  assertTrue((ttw == DateTuple{2000, 1, 30, 15*16, ZoneContext::kSuffixW}));
  assertTrue((tts == DateTuple{2000, 1, 30, 15*12, ZoneContext::kSuffixS}));
  assertTrue((ttu == DateTuple{2000, 1, 30, 15*4, ZoneContext::kSuffixU}));

  tt = {2000, 1, 30, 15*12, ZoneContext::kSuffixS};
  ExtendedZoneProcessor::expandDateTuple(
      &tt, offsetMinutes, deltaMinutes,
      &ttw, &tts, &ttu);
  assertTrue((ttw == DateTuple{2000, 1, 30, 15*16, ZoneContext::kSuffixW}));
  assertTrue((tts == DateTuple{2000, 1, 30, 15*12, ZoneContext::kSuffixS}));
  assertTrue((ttu == DateTuple{2000, 1, 30, 15*4, ZoneContext::kSuffixU}));

  tt = {2000, 1, 30, 15*4, ZoneContext::kSuffixU};
  ExtendedZoneProcessor::expandDateTuple(
      &tt, offsetMinutes, deltaMinutes,
      &ttw, &tts, &ttu);
  assertTrue((ttw == DateTuple{2000, 1, 30, 15*16, ZoneContext::kSuffixW}));
  assertTrue((tts == DateTuple{2000, 1, 30, 15*12, ZoneContext::kSuffixS}));
  assertTrue((ttu == DateTuple{2000, 1, 30, 15*4, ZoneContext::kSuffixU}));
}

//---------------------------------------------------------------------------
// Step 2B: Pass 3
//---------------------------------------------------------------------------

test(ExtendedZoneProcessorTest, compareTransitionToMatch) {
  using ace_time::extended::MatchStatus;

  // UNTIL = 2002-01-02T03:00
  const ZoneEra ERA ACE_TIME_PROGMEM = {
      nullptr /*zonePolicy*/,
      "" /*format*/,
      0 /*offsetCode*/,
      0 /*deltaCode*/,
      2002 /*untilYear*/,
      1 /*untilMonth*/,
      2 /*untilDay*/,
      12 /*untilTimeCode*/,
      ZoneContext::kSuffixW
  };

  const DateTuple EMPTY_DATE = {0, 0, 0, 0, 0};

  // MatchingEra=[2000-01-01, 2001-01-01)
  const ExtendedZoneProcessor::MatchingEra match = {
    {0, 1, 1, 0, ZoneContext::kSuffixW} /*startDateTime*/,
    {1, 1, 1, 0, ZoneContext::kSuffixW} /*untilDateTime*/,
    ZoneEraBroker(&ERA) /*era*/,
    nullptr /*prevMatch*/,
    0 /*lastOffsetMinutes*/,
    0 /*lastDeltaMinutes*/
  };

  // transitionTime = 1999-12-31
  ExtendedZoneProcessor::Transition transition0 = {
    &match /*match*/,
    ZoneRuleBroker(nullptr) /*rule*/,
    {-1, 12, 31, 0, ZoneContext::kSuffixW} /*transitionTime*/,
    EMPTY_DATE, EMPTY_DATE,
  #if ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG
    EMPTY_DATE,
  #endif
    0, 0, 0, {0}, {0}, false
  };

  // transitionTime = 2000-01-01
  ExtendedZoneProcessor::Transition transition1 = {
    &match /*match*/,
    ZoneRuleBroker(nullptr) /*rule*/,
    {0, 1, 1, 0, ZoneContext::kSuffixW} /*transitionTime*/,
    EMPTY_DATE, EMPTY_DATE,
  #if ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG
    EMPTY_DATE,
  #endif
    0, 0, 0, {0}, {0}, false
  };

  // transitionTime = 2000-01-02
  ExtendedZoneProcessor::Transition transition2 = {
    &match /*match*/,
    ZoneRuleBroker(nullptr) /*rule*/,
    {0, 1, 2, 0, ZoneContext::kSuffixW} /*transitionTime*/,
    EMPTY_DATE, EMPTY_DATE,
  #if ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG
    EMPTY_DATE,
  #endif
    0, 0, 0, {0}, {0}, false
  };

  // transitionTime = 2001-02-03
  ExtendedZoneProcessor::Transition transition3 = {
    &match /*match*/,
    ZoneRuleBroker(nullptr) /*rule*/,
    {1, 2, 3, 0, ZoneContext::kSuffixW} /*transitionTime*/,
    EMPTY_DATE, EMPTY_DATE,
  #if ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG
    EMPTY_DATE,
  #endif
    0, 0, 0, {0}, {0}, false
  };

  ExtendedZoneProcessor::Transition* transitions[] = {
    &transition0,
    &transition1,
    &transition2,
    &transition3,
  };

  // Populate the transitionTimeS and transitionTimeU fields.
  ExtendedZoneProcessor::fixTransitionTimes(&transitions[0], &transitions[4]);

  assertEqual(
      (uint8_t) MatchStatus::kPrior,
      (uint8_t) ExtendedZoneProcessor::compareTransitionToMatch(
          &transition0, &match)
  );

  assertEqual(
      (uint8_t) MatchStatus::kExactMatch,
      (uint8_t) ExtendedZoneProcessor::compareTransitionToMatch(
          &transition1, &match)
  );

  assertEqual(
      (uint8_t) MatchStatus::kWithinMatch,
      (uint8_t) ExtendedZoneProcessor::compareTransitionToMatch(
          &transition2, &match)
  );

  assertEqual(
      (uint8_t) MatchStatus::kFarFuture,
      (uint8_t) ExtendedZoneProcessor::compareTransitionToMatch(
          &transition3, &match)
  );
}

test(ExtendedZoneProcessorTest, processTransitionMatchStatus) {
  using ace_time::extended::MatchStatus;

  // UNTIL = 2002-01-02T03:00
  const ZoneEra ERA ACE_TIME_PROGMEM = {
      nullptr /*zonePolicy*/,
      "" /*format*/,
      0 /*offsetCode*/,
      0 /*deltaCode*/,
      2002 /*untilYear*/,
      1 /*untilMonth*/,
      2 /*untilDay*/,
      12 /*untilTimeCode*/,
      ZoneContext::kSuffixW
  };

  const DateTuple EMPTY_DATE = {0, 0, 0, 0, 0};

  // [2000-01-01, 2001-01-01)
  ExtendedZoneProcessor::Transition* prior = nullptr;
  const ExtendedZoneProcessor::MatchingEra match = {
    {0, 1, 1, 0, ZoneContext::kSuffixW} /*startDateTime*/,
    {1, 1, 1, 0, ZoneContext::kSuffixW} /*untilDateTime*/,
    ZoneEraBroker(&ERA) /*era*/,
    nullptr /*prevMatch*/,
    0 /*lastOffsetMinutes*/,
    0 /*lastDeltaMinutes*/
  };

  // This transition occurs before the match, so prior should be filled.
  // transitionTime = 1999-12-31
  ExtendedZoneProcessor::Transition transition0 = {
    &match /*match*/,
    ZoneRuleBroker(nullptr) /*rule*/,
    {-1, 12, 31, 0, ZoneContext::kSuffixW} /*transitionTime*/,
    EMPTY_DATE, EMPTY_DATE,
  #if ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG
    EMPTY_DATE,
  #endif
    0, 0, 0, {0}, {0}, false
  };

  // This occurs at exactly match.startDateTime, so should replace the prior.
  // transitionTime = 2000-01-01
  ExtendedZoneProcessor::Transition transition1 = {
    &match /*match*/,
    ZoneRuleBroker(nullptr) /*rule*/,
    {0, 1, 1, 0, ZoneContext::kSuffixW} /*transitionTime*/,
    EMPTY_DATE, EMPTY_DATE,
  #if ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG
    EMPTY_DATE,
  #endif
    0, 0, 0, {0}, {0}, false
  };

  // An interior transition. Prior should not change.
  // transitionTime = 2000-01-02
  ExtendedZoneProcessor::Transition transition2 = {
    &match /*match*/,
    ZoneRuleBroker(nullptr) /*rule*/,
    {0, 1, 2, 0, ZoneContext::kSuffixW} /*transitionTime*/,
    EMPTY_DATE, EMPTY_DATE,
  #if ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG
    EMPTY_DATE,
  #endif
    0, 0, 0, {0}, {0}, false
  };

  // Occurs after match.untilDateTime, so should be rejected.
  // transitionTime = 2001-01-02
  ExtendedZoneProcessor::Transition transition3 = {
    &match /*match*/,
    ZoneRuleBroker(nullptr) /*rule*/,
    {1, 1, 2, 0, ZoneContext::kSuffixW} /*transitionTime*/,
    EMPTY_DATE, EMPTY_DATE,
  #if ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG
    EMPTY_DATE,
  #endif
    0, 0, 0, {0}, {0}, false
  };

  ExtendedZoneProcessor::Transition* transitions[] = {
    &transition0,
    &transition1,
    &transition2,
    &transition3,
  };

  // Populate the transitionTimeS and transitionTimeU fields.
  ExtendedZoneProcessor::fixTransitionTimes(&transitions[0], &transitions[4]);

  ExtendedZoneProcessor::processTransitionMatchStatus(&transition0, &prior);
  assertEqual(
      (uint8_t) MatchStatus::kPrior,
      (uint8_t) transition0.matchStatus
  );
  assertEqual(prior, &transition0);

  ExtendedZoneProcessor::processTransitionMatchStatus(&transition1, &prior);
  assertEqual(
      (uint8_t) MatchStatus::kExactMatch,
      (uint8_t) transition1.matchStatus
  );
  assertEqual(prior, &transition1);

  ExtendedZoneProcessor::processTransitionMatchStatus(&transition2, &prior);
  assertEqual(
      (uint8_t) MatchStatus::kWithinMatch,
      (uint8_t) transition2.matchStatus
  );
  assertEqual(prior, &transition1);

  ExtendedZoneProcessor::processTransitionMatchStatus(&transition3, &prior);
  assertEqual(
      (uint8_t) MatchStatus::kFarFuture,
      (uint8_t) transition3.matchStatus
  );
  assertEqual(prior, &transition1);
}

//---------------------------------------------------------------------------
// Step 2B
//---------------------------------------------------------------------------

test(ExtendedZoneProcessorTest, createTransitionsFromNamedMatch) {
  ExtendedZoneProcessor::MatchingEra match = {
    {2018, 12, 1, 0, ZoneContext::kSuffixW},
    {2020, 2, 1, 0, ZoneContext::kSuffixW},
    ZoneEraBroker(&kZoneEraTestLos_Angeles[0]),
    nullptr /*prevMatch*/,
    0 /*lastOffsetMinutes*/,
    0 /*lastDeltaMinutes*/
  };

  // Reserve storage for the Transitions
  ExtendedZoneProcessor::TransitionStorage storage;
  storage.init();

  ExtendedZoneProcessor::createTransitionsFromNamedMatch(storage, &match);
  assertEqual(3,
      (int) (storage.getActivePoolEnd() - storage.getActivePoolBegin()));
  ExtendedZoneProcessor::Transition** t = storage.getActivePoolBegin();
  assertTrue(((*t++)->transitionTime == DateTuple{2018, 12, 1, 0,
      ZoneContext::kSuffixW}));
  assertTrue(((*t++)->transitionTime == DateTuple{2019, 3, 10, 15*8,
      ZoneContext::kSuffixW}));
  assertTrue(((*t++)->transitionTime == DateTuple{2019, 11, 3, 15*8,
      ZoneContext::kSuffixW}));
}

//---------------------------------------------------------------------------
// Step 3, Step 4
//---------------------------------------------------------------------------

test(ExtendedZoneProcessorTest, fixTransitionTimes_generateStartUntilTimes) {
  using ace_time::extended::MatchStatus;

  // Create 3 matches for the AlmostLosAngeles test zone.
  YearMonthTuple startYm = {2018, 12};
  YearMonthTuple untilYm = {2020, 2};
  const uint8_t kMaxMatches = 4;
  ExtendedZoneProcessor::MatchingEra matches[kMaxMatches];
  uint8_t numMatches = ExtendedZoneProcessor::findMatches(
      ZoneInfoBroker(&kZoneAlmostLosAngeles), startYm, untilYm,
      matches, kMaxMatches);
  assertEqual(3, numMatches);

  // Create a custom template instantiation to use a different SIZE than the
  // pre-defined typedef in ExtendedZoneProcess::TransitionStorage.
  TransitionStorageTemplate<
      4 /*SIZE*/,
      ZoneEraBroker,
      ZonePolicyBroker,
      ZoneRuleBroker> storage;
  storage.init();

  // Create 3 Transitions corresponding to the matches.
  // Implements ExtendedZoneProcessor::createTransitionsFromSimpleMatch().
  ExtendedZoneProcessor::Transition* transition1 = storage.getFreeAgent();
  ExtendedZoneProcessor::createTransitionForYear(
      transition1, 2000 /*year, not used*/, ZoneRuleBroker(nullptr) /*rule*/,
      &matches[0]);
  transition1->matchStatus = MatchStatus::kExactMatch; // synthetic example
  storage.addFreeAgentToCandidatePool();

  ExtendedZoneProcessor::Transition* transition2 = storage.getFreeAgent();
  ExtendedZoneProcessor::createTransitionForYear(
      transition2, 2000 /*year, not used*/, ZoneRuleBroker(nullptr) /*rule*/,
      &matches[1]);
  transition2->matchStatus = MatchStatus::kWithinMatch; // synthetic example
  storage.addFreeAgentToCandidatePool();

  ExtendedZoneProcessor::Transition* transition3 = storage.getFreeAgent();
  ExtendedZoneProcessor::createTransitionForYear(
      transition3, 2000 /*year, not used*/, ZoneRuleBroker(nullptr) /*rule*/,
      &matches[2]);
  transition3->matchStatus = MatchStatus::kWithinMatch; // synthetic example
  storage.addFreeAgentToCandidatePool();

  // Move actives to Active pool.
  storage.addActiveCandidatesToActivePool();
  ExtendedZoneProcessor::Transition** begin = storage.getActivePoolBegin();
  ExtendedZoneProcessor::Transition** end = storage.getActivePoolEnd();
  assertEqual(3, (int) (end - begin));
  assertTrue(begin[0] == transition1);
  assertTrue(begin[1] == transition2);
  assertTrue(begin[2] == transition3);

  // Chain the transitions.
  ExtendedZoneProcessor::fixTransitionTimes(begin, end);

  // Verify. The first Transition is extended to -infinity.
  assertTrue((transition1->transitionTime == DateTuple{2018, 12, 1, 0,
      ZoneContext::kSuffixW}));
  assertTrue((transition1->transitionTimeS == DateTuple{2018, 12, 1, 0,
      ZoneContext::kSuffixS}));
  assertTrue((transition1->transitionTimeU == DateTuple{2018, 12, 1, 15*32,
      ZoneContext::kSuffixU}));

  // Second transition uses the UTC offset of the first.
  assertTrue((transition2->transitionTime == DateTuple{2019, 3, 10, 15*8,
      ZoneContext::kSuffixW}));
  assertTrue((transition2->transitionTimeS == DateTuple{2019, 3, 10, 15*8,
      ZoneContext::kSuffixS}));
  assertTrue((transition2->transitionTimeU == DateTuple{2019, 3, 10, 15*40,
      ZoneContext::kSuffixU}));

  // Third transition uses the UTC offset of the second.
  assertTrue((transition3->transitionTime == DateTuple{2019, 11, 3, 15*8,
      ZoneContext::kSuffixW}));
  assertTrue((transition3->transitionTimeS == DateTuple{2019, 11, 3, 15*4,
      ZoneContext::kSuffixS}));
  assertTrue((transition3->transitionTimeU == DateTuple{2019, 11, 3, 15*36,
      ZoneContext::kSuffixU}));

  // Generate the startDateTime and untilDateTime of the transitions.
  ExtendedZoneProcessor::generateStartUntilTimes(begin, end);

  // Verify. The first transition startTime should be the same as its
  // transitionTime.
  assertTrue((transition1->startDateTime == DateTuple{2018, 12, 1, 0,
      ZoneContext::kSuffixW}));
  assertTrue((transition1->untilDateTime == DateTuple{2019, 3, 10, 15*8,
      ZoneContext::kSuffixW}));
  acetime_t epochSecs = OffsetDateTime::forComponents(
      2018, 12, 1, 0, 0, 0, TimeOffset::forHours(-8)).toEpochSeconds();
  assertEqual(epochSecs, transition1->startEpochSeconds);

  // Second transition startTime is shifted forward one hour into PDT.
  assertTrue((transition2->startDateTime == DateTuple{2019, 3, 10, 15*12,
      ZoneContext::kSuffixW}));
  assertTrue((transition2->untilDateTime == DateTuple{2019, 11, 3, 15*8,
      ZoneContext::kSuffixW}));
  epochSecs = OffsetDateTime::forComponents(
      2019, 3, 10, 3, 0, 0, TimeOffset::forHours(-7)).toEpochSeconds();
  assertEqual(epochSecs, transition2->startEpochSeconds);

  // Third transition startTime is shifted back one hour into PST.
  assertTrue((transition3->startDateTime == DateTuple{2019, 11, 3, 15*4,
      ZoneContext::kSuffixW}));
  assertTrue((transition3->untilDateTime == DateTuple{2020, 2, 1, 0,
      ZoneContext::kSuffixW}));
  epochSecs = OffsetDateTime::forComponents(
      2019, 11, 3, 1, 0, 0, TimeOffset::forHours(-8)).toEpochSeconds();
  assertEqual(epochSecs, transition3->startEpochSeconds);
}

//---------------------------------------------------------------------------
// Step 5
//---------------------------------------------------------------------------

test(ExtendedZoneProcessorTest, createAbbreviation) {
  const uint8_t kDstSize = 6;
  char dst[kDstSize];

  // If no '%', deltaMinutes and letter should not matter
  ExtendedZoneProcessor::createAbbreviation(dst, kDstSize, "SAST", 0, nullptr);
  assertEqual("SAST", dst);

  ExtendedZoneProcessor::createAbbreviation(dst, kDstSize, "SAST", 60, "A");
  assertEqual("SAST", dst);

  // If '%', and letter is (incorrectly) set to '\0', just copy the thing
  ExtendedZoneProcessor::createAbbreviation(dst, kDstSize, "SA%ST", 0, nullptr);
  assertEqual("SA%ST", dst);

  // If '%', then replaced with (non-null) letterString.
  ExtendedZoneProcessor::createAbbreviation(dst, kDstSize, "P%T", 60, "D");
  assertEqual("PDT", dst);

  ExtendedZoneProcessor::createAbbreviation(dst, kDstSize, "P%T", 0, "S");
  assertEqual("PST", dst);

  ExtendedZoneProcessor::createAbbreviation(dst, kDstSize, "P%T", 0, "");
  assertEqual("PT", dst);

  ExtendedZoneProcessor::createAbbreviation(dst, kDstSize, "%", 60, "CAT");
  assertEqual("CAT", dst);

  ExtendedZoneProcessor::createAbbreviation(dst, kDstSize, "%", 0, "WAT");
  assertEqual("WAT", dst);

  // If '/', then deltaMinutes selects the first or second component.
  ExtendedZoneProcessor::createAbbreviation(dst, kDstSize, "GMT/BST", 0, "");
  assertEqual("GMT", dst);

  ExtendedZoneProcessor::createAbbreviation(dst, kDstSize, "GMT/BST", 60, "");
  assertEqual("BST", dst);

  // test truncation to kDstSize
  ExtendedZoneProcessor::createAbbreviation(dst, kDstSize, "P%T3456", 60, "DD");
  assertEqual("PDDT3", dst);
}

//---------------------------------------------------------------------------
// DateTuple.
//---------------------------------------------------------------------------

test(ExtendedZoneProcessorTest, dateTupleOperatorLessThan) {
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

test(ExtendedZoneProcessorTest, dateTupleOperatorEquals) {
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

test(ExtendedZoneProcessorTest, normalizeDateTuple) {
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

test(ExtendedZoneProcessorTest, substractDateTuple) {
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

//---------------------------------------------------------------------------
// Test high level public methods of ExtendedZoneProcessor.
//---------------------------------------------------------------------------

test(ExtendedZoneProcessorTest, setZoneKey) {
  ExtendedZoneProcessor zoneProcessor(&zonedbx::kZoneAmerica_Los_Angeles);
  zoneProcessor.getUtcOffset(0);
  assertTrue(zoneProcessor.mIsFilled);

  zoneProcessor.setZoneKey((uintptr_t) &zonedbx::kZoneAustralia_Darwin);
  assertFalse(zoneProcessor.mIsFilled);
  zoneProcessor.getUtcOffset(0);
  assertTrue(zoneProcessor.mIsFilled);

  // Check that the cache remains valid if the zoneInfo does not change
  zoneProcessor.setZoneKey((uintptr_t) &zonedbx::kZoneAustralia_Darwin);
  assertTrue(zoneProcessor.mIsFilled);
}

// https://www.timeanddate.com/time/zone/usa/los-angeles
test(ExtendedZoneProcessorTest, Los_Angeles) {
  ExtendedZoneProcessor zoneProcessor(&zonedbx::kZoneAmerica_Los_Angeles);

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

test(ExtendedZoneProcessorTest, Los_Angeles_outOfBounds) {
  ExtendedZoneProcessor zoneProcessor(&zonedbx::kZoneAmerica_Los_Angeles);
  OffsetDateTime dt;
  acetime_t epochSeconds;

  assertEqual(2000, zonedbx::kZoneContext.startYear);
  assertEqual(10000, zonedbx::kZoneContext.untilYear);

  // 1998 > LocalDate::kMinYear so dt is valid, and
  dt = OffsetDateTime::forComponents(1998, 3, 11, 1, 59, 59,
      TimeOffset::forHours(-8));
  assertFalse(dt.isError());
  epochSeconds = dt.toEpochSeconds();
  // 1998 is within roughly 50 years of Epoch::currentEpochYear() of 2050
  // so toEpochSeconds() still works.
  assertNotEqual(epochSeconds, LocalDate::kInvalidEpochSeconds);
  // 1998 < ZoneContext.startYear, so getUtcOffset() fails
  assertTrue(zoneProcessor.getUtcOffset(epochSeconds).isError());
  // 1998 < ZoneContext.startYear, so getDeltaOffset() fails
  assertTrue(zoneProcessor.getDeltaOffset(epochSeconds).isError());
  // getAbbrev() returns "" on lookup failure
  assertEqual("", zoneProcessor.getAbbrev(epochSeconds));

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
// Test that getOffsetDateTime(acetime_t) returns correct fold parameter.
//---------------------------------------------------------------------------

test(ExtendedZoneProcessorTest, forEpochSeconds_during_fall_back) {
  ExtendedZoneProcessor zoneProcessor(&zonedbx::kZoneAmerica_Los_Angeles);

  // Start our sampling at 01:29:00-07:00, which is 31 minutes before the DST
  // fall-back.
  OffsetDateTime odt = OffsetDateTime::forComponents(
      2022, 11, 6, 1, 29, 0, TimeOffset::forHours(-7));
  acetime_t epochSeconds = odt.toEpochSeconds();

  // Verify fold==0 because this is the first time we're seeing this datetime.
  OffsetDateTime observed = zoneProcessor.getOffsetDateTime(epochSeconds);
  assertTrue(
      OffsetDateTime::forComponents(
          2022, 11, 6, 1, 29, 0, TimeOffset::forHours(-7))
      == observed
  );
  assertEqual(0, observed.fold());

  // 30 minutes later, we are at 01:59:00-07:00, a minute before fall-back, and
  // fold should be 0 because this is the first time seeing the datetime.
  epochSeconds += 1800;
  observed = zoneProcessor.getOffsetDateTime(epochSeconds);
  assertTrue(
      OffsetDateTime::forComponents(
          2022, 11, 6, 1, 59, 0, TimeOffset::forHours(-7))
      == observed
  );
  assertEqual(0, observed.fold());

  // 30 minutes into the overlap, we have either 02:29:00-07:00 or
  // 01:29:00-08:00. DST fall-back has occurred, so ExtendedZoneProcessor should
  // return 01:29:00-08:00, but with fold==1 because it's the second time we are
  // seeing this datetime.
  epochSeconds += 1800;
  observed = zoneProcessor.getOffsetDateTime(epochSeconds);
  assertTrue(
      OffsetDateTime::forComponents(
          2022, 11, 6, 1, 29, 0, TimeOffset::forHours(-8))
      == observed
  );
  assertEqual(1, observed.fold());

  // Another 30 minutes into the overlap, we have either 02:59:00-07:00 or
  // 01:59:00-08:00. ExtendedZoneProcessor should return 01:59:00-08:00, but
  // with fold==1 because we are seeing this datetime a second time.
  epochSeconds += 1800;
  observed = zoneProcessor.getOffsetDateTime(epochSeconds);
  assertTrue(
      OffsetDateTime::forComponents(
          2022, 11, 6, 1, 59, 0, TimeOffset::forHours(-8))
      == observed
  );
  assertEqual(1, observed.fold());

  // One more minute into the overlap, we have either 03:00:00-07:00 or
  // 02:00:00-08:00. ExtendedZoneProcessor should return 02:00:00-08:00,
  // with fold==0 because 02:00:00 was the exact point of fall-back and never
  // occurred twice.
  epochSeconds += 60;
  observed = zoneProcessor.getOffsetDateTime(epochSeconds);
  assertTrue(
      OffsetDateTime::forComponents(
          2022, 11, 6, 2, 0, 0, TimeOffset::forHours(-8))
      == observed
  );
  assertEqual(0, observed.fold());
}

test(ExtendedZoneProcessorTest, forEpochSeconds_during_spring_forward) {
  ExtendedZoneProcessor zoneProcessor(&zonedbx::kZoneAmerica_Los_Angeles);

  // Start our sampling at 01:29:00-08:00, which is 31 minutes before the DST
  // spring-forward.
  OffsetDateTime odt = OffsetDateTime::forComponents(
      2022, 3, 13, 1, 29, 0, TimeOffset::forHours(-8));
  acetime_t epochSeconds = odt.toEpochSeconds();

  // Verify fold==0 always, because spring-forward never causes repeats.
  OffsetDateTime observed = zoneProcessor.getOffsetDateTime(epochSeconds);
  assertTrue(
      OffsetDateTime::forComponents(
          2022, 3, 13, 1, 29, 0, TimeOffset::forHours(-8))
      == observed
  );
  assertEqual(0, observed.fold());

  // 30 minutes later, we are at 01:59:00-07:00, a minute before spring-forward.
  epochSeconds += 1800;
  observed = zoneProcessor.getOffsetDateTime(epochSeconds);
  assertTrue(
      OffsetDateTime::forComponents(
          2022, 3, 13, 1, 59, 0, TimeOffset::forHours(-8))
      == observed
  );
  assertEqual(0, observed.fold());

  // One minute later, we are at 02:00:00-08:00, which immediately turns into
  // 03:00:00-07:00.
  epochSeconds += 60;
  observed = zoneProcessor.getOffsetDateTime(epochSeconds);
  assertTrue(
      OffsetDateTime::forComponents(
          2022, 3, 13, 3, 0, 0, TimeOffset::forHours(-7))
      == observed
  );
  assertEqual(0, observed.fold());
}

//---------------------------------------------------------------------------
// Test that getOffsetDateTime(const LocalDateTime&) handles fold parameter
// correctly.
//---------------------------------------------------------------------------

test(ExtendedZoneProcessorTest, forComponents_during_fall_back) {
  ExtendedZoneProcessor zoneProcessor(&zonedbx::kZoneAmerica_Los_Angeles);

  // 01:29:00, before fall-back
  {
    LocalDateTime ldt = LocalDateTime::forComponents(
        2022, 11, 6, 1, 29, 0, 0 /*fold*/);
    OffsetDateTime observed = zoneProcessor.getOffsetDateTime(ldt);
    assertTrue(
        OffsetDateTime::forComponents(
            2022, 11, 6, 1, 29, 0, TimeOffset::forHours(-7))
        == observed
    );

    // Verify fold remains unchanged.
    assertEqual(0, observed.fold());
  }

  // 01:29:00, after fall-back
  {
    LocalDateTime ldt = LocalDateTime::forComponents(
        2022, 11, 6, 1, 29, 0, 1 /*fold*/);
    OffsetDateTime observed = zoneProcessor.getOffsetDateTime(ldt);
    assertTrue(
        OffsetDateTime::forComponents(
            2022, 11, 6, 1, 29, 0, TimeOffset::forHours(-8))
        == observed
    );

    // Verify fold remains unchanged.
    assertEqual(1, observed.fold());
  }
}

test(ExtendedZoneProcessorTest, forComponents_during_spring_forward) {
  ExtendedZoneProcessor zoneProcessor(&zonedbx::kZoneAmerica_Los_Angeles);

  // 02:29:00 in gap, fold==0, uses earlier transition, so maps to the later UTC
  // time.
  {
    LocalDateTime ldt = LocalDateTime::forComponents(
        2022, 3, 13, 2, 29, 0, 0 /*fold*/);
    OffsetDateTime observed = zoneProcessor.getOffsetDateTime(ldt);
    assertTrue(
        OffsetDateTime::forComponents(
            2022, 3, 13, 3, 29, 0, TimeOffset::forHours(-7))
        == observed
    );

    // Verify that fold has flipped.
    assertEqual(1, observed.fold());
  }

  // 02:29:00 in gap, fold==1, uses later transition, so maps to the earlier UTC
  // time.
  {
    LocalDateTime ldt = LocalDateTime::forComponents(
        2022, 3, 13, 2, 29, 0, 1 /*fold*/);
    OffsetDateTime observed = zoneProcessor.getOffsetDateTime(ldt);
    assertTrue(
        OffsetDateTime::forComponents(
            2022, 3, 13, 1, 29, 0, TimeOffset::forHours(-8))
        == observed
    );

    // Verify that fold has flipped.
    assertEqual(0, observed.fold());
  }
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
