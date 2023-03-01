#line 2 "ExtendedZoneProcessorTest.ino"

#include <AUnitVerbose.h>
#include <AceCommon.h> // PrintStr<>
#include <AceTime.h>
#include <ace_time/testing/EpochYearContext.h>
#include <tzonedbx/zone_policies.h>
#include <tzonedbx/zone_infos.h>

using ace_common::PrintStr;
using namespace ace_time;
using ace_time::internal::ZoneContext;
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
using ace_time::testing::EpochYearContext;
using ace_time::tzonedbx::kZoneContext;
using ace_time::tzonedbx::kZoneAmerica_Los_Angeles;
using ace_time::tzonedbx::kZonePolicyUS;
using ace_time::tzonedbx::kZoneAustralia_Darwin;
using ace_time::tzonedbx::kZoneAmerica_Caracas;

//---------------------------------------------------------------------------
// Step 1
//---------------------------------------------------------------------------

test(ExtendedZoneProcessorTest, compareEraToYearMonth) {
  static const ZoneEra ERA ACE_TIME_PROGMEM =
      {nullptr, "", 0, 0, 2000, 1, 2, 12, ZoneContext::kSuffixW};

  assertEqual(1, ExtendedZoneProcessor::compareEraToYearMonth(
      ZoneEraBroker(&kZoneContext, &ERA), 2000, 1));
  assertEqual(1, ExtendedZoneProcessor::compareEraToYearMonth(
      ZoneEraBroker(&kZoneContext, &ERA), 2000, 1));
  assertEqual(-1, ExtendedZoneProcessor::compareEraToYearMonth(
      ZoneEraBroker(&kZoneContext, &ERA), 2000, 2));
  assertEqual(-1, ExtendedZoneProcessor::compareEraToYearMonth(
      ZoneEraBroker(&kZoneContext, &ERA), 2000, 3));
}

test(ExtendedZoneProcessorTest, compareEraToYearMonth2) {
  static const ZoneEra ERA ACE_TIME_PROGMEM =
      {nullptr, "", 0, 0, 2000, 1, 0, 0, ZoneContext::kSuffixW};

  assertEqual(0, ExtendedZoneProcessor::compareEraToYearMonth(
      ZoneEraBroker(&kZoneContext, &ERA), 2000, 1));
}

test(ExtendedZoneProcessorTest, createMatchingEra) {
  // UNTIL = 2000-12-02 3:00
  static const ZoneEra era1 ACE_TIME_PROGMEM =
      {nullptr, "", 0, 0, 2000 /*y*/, 12/*m*/, 2/*d*/, 3*(60/15),
      ZoneContext::kSuffixW};

  // UNTIL = 2001-02-03 4:00
  static const ZoneEra era2 ACE_TIME_PROGMEM =
      {nullptr, "", 0, 0, 2001/*y*/, 2/*m*/, 3/*d*/, 4*(60/15),
      ZoneContext::kSuffixW};

  // UNTIL = 2002-10-11 4:00
  static const ZoneEra era3 ACE_TIME_PROGMEM =
      {nullptr, "", 0, 0, 2002/*y*/, 10/*m*/, 11/*d*/, 4*(60/15),
      ZoneContext::kSuffixW};

  // 14-month interval, from 2000-12 until 2002-02
  YearMonthTuple startYm = {2000, 12};
  YearMonthTuple untilYm = {2002, 2};

  // No previous matching era, so startDateTime is set to startYm.
  ExtendedZoneProcessor::MatchingEra match1 =
      ExtendedZoneProcessor::createMatchingEra(
          nullptr /*prevMatch*/,
          ZoneEraBroker(&kZoneContext, &era1) /*era*/,
          startYm,
          untilYm);
  assertTrue((match1.startDateTime == DateTuple{2000, 12, 1, 60*0,
      ZoneContext::kSuffixW}));
  assertTrue((match1.untilDateTime == DateTuple{2000, 12, 2, 60*3,
      ZoneContext::kSuffixW}));
  assertTrue(match1.era.equals(ZoneEraBroker(&kZoneContext, &era1)));

  // startDateTime is set to the prevMatch.untilDateTime.
  // untilDateTime is < untilYm, so is retained.
  ExtendedZoneProcessor::MatchingEra match2 =
      ExtendedZoneProcessor::createMatchingEra(
          &match1,
          ZoneEraBroker(&kZoneContext, &era2) /*era*/,
          startYm,
          untilYm);
  assertTrue((match2.startDateTime == DateTuple{2000, 12, 2, 60*3,
      ZoneContext::kSuffixW}));
  assertTrue((match2.untilDateTime == DateTuple{2001, 2, 3, 60*4,
      ZoneContext::kSuffixW}));
  assertTrue(match2.era.equals(ZoneEraBroker(&kZoneContext, &era2)));

  // startDateTime is set to the prevMatch.untilDateTime.
  // untilDateTime is > untilYm so truncated to untilYm.
  ExtendedZoneProcessor::MatchingEra match3 =
      ExtendedZoneProcessor::createMatchingEra(
          &match2,
          ZoneEraBroker(&kZoneContext, &era3) /*era*/,
          startYm,
          untilYm);
  assertTrue((match3.startDateTime == DateTuple{2001, 2, 3, 60*4,
      ZoneContext::kSuffixW}));
  assertTrue((match3.untilDateTime == DateTuple{2002, 2, 1, 60*0,
      ZoneContext::kSuffixW}));
  assertTrue(match3.era.equals(ZoneEraBroker(&kZoneContext, &era3)));
}

// Validate findMatches() for simple eras, using one of the zones that has only
// simple eras, such as:
//
//  * Africa/Sao_Tome (+00)
//  * America/Caracas (-04, -04:30)
//  * America/Eirunepe (-05, -04)
//  * America/Rio_Branco (-05, -04)
//  * America/Santarem (-04, -03)
//  * Antarctica/Casey (+08, +11)
//  * Antarctica/Davis (+07, +05)
//  * Antarctica/Mawson (+06, +05)
//  * Asia/Colombo (+06, +0530)
//  * Asia/Dili (+08, +09)
//  * Asia/Pyongyang (+09, +9:30)
//  * Pacific/Bougainville (+10, +11)
//  * Pacific/Fakaofo (-11, +13)
//  * Pacific/Kosrae (+12, +11)
test(ExtendedZoneProcessorTest, findMatches_simple) {
  YearMonthTuple startYm = {2015, 12};
  YearMonthTuple untilYm = {2017, 2};
  const uint8_t kMaxMatches = 4;
  ExtendedZoneProcessor::MatchingEra matches[kMaxMatches];

  // America/Caracas has 3 simple eras with the following UNTIL: 2007, 2016,
  // 10000. The interval [2015/12, 2017/2] should return 4 transitions.
  uint8_t numMatches = ExtendedZoneProcessor::findMatches(
      ZoneInfoBroker(&kZoneAmerica_Caracas), startYm, untilYm,
      matches, kMaxMatches);
  assertEqual(2, numMatches);

  const auto* eras = kZoneAmerica_Caracas.eras;

  // matches[0] maps to eras[1]
  assertTrue((matches[0].startDateTime == DateTuple{2015, 12, 1, 0,
      ZoneContext::kSuffixW}));
  assertTrue((matches[0].untilDateTime == DateTuple{2016, 5, 1, 15*10,
      ZoneContext::kSuffixW}));
  assertTrue(matches[0].era.equals(ZoneEraBroker(&kZoneContext, &eras[1])));

  // matches[1] maps to eras[2]
  assertTrue((matches[1].startDateTime == DateTuple{2016, 5, 1, 15*10,
      ZoneContext::kSuffixW}));
  assertTrue((matches[1].untilDateTime == DateTuple{2017, 2, 1, 0,
      ZoneContext::kSuffixW}));
  assertTrue(matches[1].era.equals(ZoneEraBroker(&kZoneContext, &eras[2])));
}

test(ExtendedZoneProcessorTest, findMatches_named) {
  YearMonthTuple startYm = {2018, 12};
  YearMonthTuple untilYm = {2020, 2};
  const uint8_t kMaxMatches = 4;
  ExtendedZoneProcessor::MatchingEra matches[kMaxMatches];
  uint8_t numMatches = ExtendedZoneProcessor::findMatches(
      ZoneInfoBroker(&kZoneAmerica_Los_Angeles), startYm, untilYm,
      matches, kMaxMatches);
  assertEqual(1, numMatches);

  assertTrue((matches[0].startDateTime == DateTuple{2018, 12, 1, 0,
      ZoneContext::kSuffixW}));
  assertTrue((matches[0].untilDateTime == DateTuple{2020, 2, 1, 0,
      ZoneContext::kSuffixW}));
  const auto* eras = kZoneAmerica_Los_Angeles.eras;
  assertTrue(matches[0].era.equals(ZoneEraBroker(&kZoneContext, &eras[0])));
}

//---------------------------------------------------------------------------
// Step 2A
//---------------------------------------------------------------------------

test(ExtendedZoneProcessorTest, getTransitionTime) {
  // Rule 6, [2007,9999]
  // Rule    US    2007    max    -    Nov    Sun>=1    2:00    0    S
  const auto rule = ZoneRuleBroker(&kZoneContext, &kZonePolicyUS.rules[6]);

  // Nov 4 2018
  DateTuple dt = ExtendedZoneProcessor::getTransitionTime(2018, rule);
  assertTrue((dt == DateTuple{2018, 11, 4, 15*8, ZoneContext::kSuffixW}));

  // Nov 3 2019
  dt = ExtendedZoneProcessor::getTransitionTime(2019, rule);
  assertTrue((dt == DateTuple{2019, 11, 3, 15*8, ZoneContext::kSuffixW}));
}

test(ExtendedZoneProcessorTest, createTransitionForYear) {
  const auto* eras = kZoneAmerica_Los_Angeles.eras;
  const ExtendedZoneProcessor::MatchingEra match = {
    {2018, 12, 1, 0, ZoneContext::kSuffixW},
    {2020, 2, 1, 0, ZoneContext::kSuffixW},
    ZoneEraBroker(&kZoneContext, &eras[0]),
    nullptr /*prevMatch*/,
    0 /*lastOffsetMinutes*/,
    0 /*lastDeltaMinutes*/
  };

  // Rule 6, [2007,9999], Nov Sun>=1
  const auto rule = ZoneRuleBroker(&kZoneContext, &kZonePolicyUS.rules[6]);
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

test(ExtendedZoneProcessorTest, compareTransitionToMatchFuzzy) {
  using ace_time::extended::CompareStatus;

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
    &match /*match*/,
    {1999, 11, 1, 0, ZoneContext::kSuffixW} /*transitionTime*/,
    EMPTY_DATE, EMPTY_DATE,
  #if ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG
    EMPTY_DATE,
  #endif
    0, 0, 0, {0}, {0}, false
  };
  assertEqual(
      (uint8_t) CompareStatus::kPrior,
      (uint8_t) ExtendedZoneProcessor::compareTransitionToMatchFuzzy(
          &transition, &match));

  transition = {
    &match /*match*/,
    {1999, 12, 1, 0, ZoneContext::kSuffixW} /*transitionTime*/,
    EMPTY_DATE, EMPTY_DATE,
  #if ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG
    EMPTY_DATE,
  #endif
    0, 0, 0, {0}, {0}, false
  };
  assertEqual(
      (uint8_t) CompareStatus::kWithinMatch,
      (uint8_t) ExtendedZoneProcessor::compareTransitionToMatchFuzzy(
          &transition, &match));

  transition = {
    &match /*match*/,
    {2000, 1, 1, 0, ZoneContext::kSuffixW} /*transitionTime*/,
    EMPTY_DATE, EMPTY_DATE,
  #if ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG
    EMPTY_DATE,
  #endif
    0, 0, 0, {0}, {0}, false
  };
  assertEqual(
      (uint8_t) CompareStatus::kWithinMatch,
      (uint8_t) ExtendedZoneProcessor::compareTransitionToMatchFuzzy(
          &transition, &match));

  transition = {
    &match /*match*/,
    {2001, 1, 1, 0, ZoneContext::kSuffixW} /*transitionTime*/,
    EMPTY_DATE, EMPTY_DATE,
  #if ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG
    EMPTY_DATE,
  #endif
    0, 0, 0, {0}, {0}, false
  };
  assertEqual(
      (uint8_t) CompareStatus::kWithinMatch,
      (uint8_t) ExtendedZoneProcessor::compareTransitionToMatchFuzzy(
          &transition, &match));

  transition = {
    &match /*match*/,
    {2001, 2, 1, 0, ZoneContext::kSuffixW} /*transitionTime*/,
    EMPTY_DATE, EMPTY_DATE,
  #if ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG
    EMPTY_DATE,
  #endif
    0, 0, 0, {0}, {0}, false
  };
  assertEqual(
      (uint8_t) CompareStatus::kWithinMatch,
      (uint8_t) ExtendedZoneProcessor::compareTransitionToMatchFuzzy(
          &transition, &match));

  transition = {
    &match /*match*/,
    {2001, 3, 1, 0, ZoneContext::kSuffixW} /*transitionTime*/,
    EMPTY_DATE, EMPTY_DATE,
  #if ACE_TIME_EXTENDED_ZONE_PROCESSOR_DEBUG
    EMPTY_DATE,
  #endif
    0, 0, 0, {0}, {0}, false
  };
  assertEqual(
      (uint8_t) CompareStatus::kFarFuture,
      (uint8_t) ExtendedZoneProcessor::compareTransitionToMatchFuzzy(
          &transition, &match));
}

test(ExtendedZoneProcessorTest, findCandidateTransitions) {
  const auto* eras = kZoneAmerica_Los_Angeles.eras;
  const ExtendedZoneProcessor::MatchingEra match = {
    {2018, 12, 1, 0, ZoneContext::kSuffixW},
    {2020, 2, 1, 0, ZoneContext::kSuffixW},
    ZoneEraBroker(&kZoneContext, &eras[0]),
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
  using ace_time::extended::CompareStatus;

  // UNTIL = 2002-01-02T03:00
  static const ZoneEra ERA ACE_TIME_PROGMEM = {
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
    ZoneEraBroker(&kZoneContext, &ERA) /*era*/,
    nullptr /*prevMatch*/,
    0 /*lastOffsetMinutes*/,
    0 /*lastDeltaMinutes*/
  };

  // transitionTime = 1999-12-31
  ExtendedZoneProcessor::Transition transition0 = {
    &match /*match*/,
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
      (uint8_t) CompareStatus::kPrior,
      (uint8_t) ExtendedZoneProcessor::compareTransitionToMatch(
          &transition0, &match)
  );

  assertEqual(
      (uint8_t) CompareStatus::kExactMatch,
      (uint8_t) ExtendedZoneProcessor::compareTransitionToMatch(
          &transition1, &match)
  );

  assertEqual(
      (uint8_t) CompareStatus::kWithinMatch,
      (uint8_t) ExtendedZoneProcessor::compareTransitionToMatch(
          &transition2, &match)
  );

  assertEqual(
      (uint8_t) CompareStatus::kFarFuture,
      (uint8_t) ExtendedZoneProcessor::compareTransitionToMatch(
          &transition3, &match)
  );
}

test(ExtendedZoneProcessorTest, processTransitionCompareStatus) {
  using ace_time::extended::CompareStatus;

  // UNTIL = 2002-01-02T03:00
  static const ZoneEra ERA ACE_TIME_PROGMEM = {
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
    ZoneEraBroker(&kZoneContext, &ERA) /*era*/,
    nullptr /*prevMatch*/,
    0 /*lastOffsetMinutes*/,
    0 /*lastDeltaMinutes*/
  };

  // This transition occurs before the match, so prior should be filled.
  // transitionTime = 1999-12-31
  ExtendedZoneProcessor::Transition transition0 = {
    &match /*match*/,
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

  ExtendedZoneProcessor::processTransitionCompareStatus(&transition0, &prior);
  assertEqual(
      (uint8_t) CompareStatus::kPrior,
      (uint8_t) transition0.compareStatus
  );
  assertEqual(prior, &transition0);

  ExtendedZoneProcessor::processTransitionCompareStatus(&transition1, &prior);
  assertEqual(
      (uint8_t) CompareStatus::kExactMatch,
      (uint8_t) transition1.compareStatus
  );
  assertEqual(prior, &transition1);

  ExtendedZoneProcessor::processTransitionCompareStatus(&transition2, &prior);
  assertEqual(
      (uint8_t) CompareStatus::kWithinMatch,
      (uint8_t) transition2.compareStatus
  );
  assertEqual(prior, &transition1);

  ExtendedZoneProcessor::processTransitionCompareStatus(&transition3, &prior);
  assertEqual(
      (uint8_t) CompareStatus::kFarFuture,
      (uint8_t) transition3.compareStatus
  );
  assertEqual(prior, &transition1);
}

//---------------------------------------------------------------------------
// Step 2B
//---------------------------------------------------------------------------

test(ExtendedZoneProcessorTest, createTransitionsFromNamedMatch) {
  const auto* eras = kZoneAmerica_Los_Angeles.eras;
  ExtendedZoneProcessor::MatchingEra match = {
    {2018, 12, 1, 0, ZoneContext::kSuffixW},
    {2020, 2, 1, 0, ZoneContext::kSuffixW},
    ZoneEraBroker(&kZoneContext, &eras[0]),
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
// Step 3, Step 4. Use America/Los_Angeles to calculate the transitions
// beause I am familiar with it.
//---------------------------------------------------------------------------

test(ExtendedZoneProcessorTest, fixTransitionTimes_generateStartUntilTimes) {
  using ace_time::extended::CompareStatus;
  ExtendedZoneProcessor zoneProcessor;

  // Step 1: America/Los_Angeles matches one era, which points to US policy.
  YearMonthTuple startYm = {2017, 12};
  YearMonthTuple untilYm = {2019, 2};
  uint8_t numMatches = ExtendedZoneProcessor::findMatches(
      ZoneInfoBroker(&kZoneAmerica_Los_Angeles), startYm, untilYm,
      zoneProcessor.mMatches, zoneProcessor.kMaxMatches);
  assertEqual(1, numMatches);

  // Step 2: Create transitions.
  zoneProcessor.mTransitionStorage.init();
  ExtendedZoneProcessor::createTransitions(
      zoneProcessor.mTransitionStorage,
      zoneProcessor.mMatches,
      numMatches);

  // Step 2: Verification: there are 3 transitions:
  //  * [2017-12-01, 2018-03-07)
  //  * [2018-03-07, 2018-11-07)
  //  * [2018-11-07, 2019-02-01)
  ExtendedZoneProcessor::Transition** begin =
      zoneProcessor.mTransitionStorage.getActivePoolBegin();
  ExtendedZoneProcessor::Transition** end =
      zoneProcessor.mTransitionStorage.getActivePoolEnd();
  assertEqual(3, (int) (end - begin));
  ExtendedZoneProcessor::Transition* transition0 = begin[0];
  ExtendedZoneProcessor::Transition* transition1 = begin[1];
  ExtendedZoneProcessor::Transition* transition2 = begin[2];

  // Step 3: Chain the transitions by fixing the transition times.
  ExtendedZoneProcessor::fixTransitionTimes(begin, end);

  // Step 3: Verification: The first Transition starts at 2017-12-01.
  assertTrue((transition0->transitionTime == DateTuple{2017, 12, 1, 0,
      ZoneContext::kSuffixW}));
  assertTrue((transition0->transitionTimeS == DateTuple{2017, 12, 1, 0,
      ZoneContext::kSuffixS}));
  assertTrue((transition0->transitionTimeU == DateTuple{2017, 12, 1, 8*60,
      ZoneContext::kSuffixU}));

  // Step 3: Verification: Second transition springs forward at 2018-03-11
  // 02:00.
  assertTrue((transition1->transitionTime == DateTuple{2018, 3, 11, 2*60,
      ZoneContext::kSuffixW}));
  assertTrue((transition1->transitionTimeS == DateTuple{2018, 3, 11, 2*60,
      ZoneContext::kSuffixS}));
  assertTrue((transition1->transitionTimeU == DateTuple{2018, 3, 11, 10*60,
      ZoneContext::kSuffixU}));

  // Step 3: Verification: Third transition falls back at 2018-11-04 02:00.
  assertTrue((transition2->transitionTime == DateTuple{2018, 11, 4, 2*60,
      ZoneContext::kSuffixW}));
  assertTrue((transition2->transitionTimeS == DateTuple{2018, 11, 4, 1*60,
      ZoneContext::kSuffixS}));
  assertTrue((transition2->transitionTimeU == DateTuple{2018, 11, 4, 9*60,
      ZoneContext::kSuffixU}));

  // Step 4: Generate the startDateTime and untilDateTime of the transitions.
  ExtendedZoneProcessor::generateStartUntilTimes(begin, end);

  // Step 4: Verification: The first transition startTime should be the same as
  // its transitionTime.
  assertTrue((transition0->startDateTime == DateTuple{2017, 12, 1, 0,
      ZoneContext::kSuffixW}));
  assertTrue((transition0->untilDateTime == DateTuple{2018, 3, 11, 2*60,
      ZoneContext::kSuffixW}));
  acetime_t epochSecs = OffsetDateTime::forComponents(
      2017, 12, 1, 0, 0, 0, TimeOffset::forHours(-8)).toEpochSeconds();
  assertEqual(epochSecs, transition0->startEpochSeconds);

  // Step 4: Verification: Second transition startTime is shifted forward one
  // hour into PDT.
  assertTrue((transition1->startDateTime == DateTuple{2018, 3, 11, 3*60,
      ZoneContext::kSuffixW}));
  assertTrue((transition1->untilDateTime == DateTuple{2018, 11, 4, 2*60,
      ZoneContext::kSuffixW}));
  epochSecs = OffsetDateTime::forComponents(
      2018, 3, 11, 3, 0, 0, TimeOffset::forHours(-7)).toEpochSeconds();
  assertEqual(epochSecs, transition1->startEpochSeconds);

  // Step 4: Verification: Third transition startTime is shifted back one hour
  // into PST.
  assertTrue((transition2->startDateTime == DateTuple{2018, 11, 4, 1*60,
      ZoneContext::kSuffixW}));
  assertTrue((transition2->untilDateTime == DateTuple{2019, 2, 1, 0,
      ZoneContext::kSuffixW}));
  epochSecs = OffsetDateTime::forComponents(
      2018, 11, 4, 1, 0, 0, TimeOffset::forHours(-8)).toEpochSeconds();
  assertEqual(epochSecs, transition2->startEpochSeconds);
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
// Test high level public methods of ExtendedZoneProcessor.
//---------------------------------------------------------------------------

test(ExtendedZoneProcessorTest, setZoneKey) {
  ExtendedZoneProcessor zoneProcessor(&kZoneAmerica_Los_Angeles);
  zoneProcessor.initForEpochSeconds(0);
  assertTrue(zoneProcessor.mIsFilled);

  zoneProcessor.setZoneKey((uintptr_t) &kZoneAustralia_Darwin);
  assertFalse(zoneProcessor.mIsFilled);
  zoneProcessor.initForEpochSeconds(0);
  assertTrue(zoneProcessor.mIsFilled);

  // Check that the cache remains valid if the zoneInfo does not change
  zoneProcessor.setZoneKey((uintptr_t) &kZoneAustralia_Darwin);
  assertTrue(zoneProcessor.mIsFilled);
}

test(ExtendedZoneProcessorTest, printNameTo) {
  ExtendedZoneProcessor zoneProcessor(&kZoneAmerica_Los_Angeles);
  PrintStr<32> printStr;
  zoneProcessor.printNameTo(printStr);
  assertEqual(F("America/Los_Angeles"), printStr.cstr());
  printStr.flush();
  zoneProcessor.printShortNameTo(printStr);
  assertEqual(F("Los Angeles"), printStr.cstr());
}

//---------------------------------------------------------------------------

// Test findByEpochSeconds(). Result can be kTypeNotFound, kTypeExact,
// kTypeOverlap, but never kTypeGap.
test(ExtendedZoneProcessorTest, findByEpochSeconds) {
  ExtendedZoneProcessor zoneProcessor(&kZoneAmerica_Los_Angeles);
  OffsetDateTime dt;
  acetime_t epochSeconds;
  FindResult result;

  // 01:59:59 just before spring forward
  dt = OffsetDateTime::forComponents(2018, 3, 11, 1, 59, 59,
      TimeOffset::forHours(-8));
  epochSeconds = dt.toEpochSeconds();
  result = zoneProcessor.findByEpochSeconds(epochSeconds);
  assertEqual(result.type, FindResult::kTypeExact);
  assertEqual(result.fold, 0);
  assertEqual(-8*60, result.stdOffsetMinutes);
  assertEqual(0*60, result.dstOffsetMinutes);
  assertEqual(-8*60, result.reqStdOffsetMinutes);
  assertEqual(0*60, result.reqDstOffsetMinutes);
  assertEqual("PST", result.abbrev);

  // 02:00 spring forward
  dt = OffsetDateTime::forComponents(2018, 3, 11, 2, 0, 0,
      TimeOffset::forHours(-8));
  epochSeconds = dt.toEpochSeconds();
  result = zoneProcessor.findByEpochSeconds(epochSeconds);
  assertEqual(result.type, FindResult::kTypeExact);
  assertEqual(result.fold, 0);
  assertEqual(-8*60, result.stdOffsetMinutes);
  assertEqual(1*60, result.dstOffsetMinutes);
  assertEqual(-8*60, result.reqStdOffsetMinutes);
  assertEqual(1*60, result.reqDstOffsetMinutes);
  assertEqual("PDT", result.abbrev);

  // 01:00 before fall back, overlap, first occurrence
  dt = OffsetDateTime::forComponents(2018, 11, 4, 1, 0, 0,
      TimeOffset::forHours(-7));
  epochSeconds = dt.toEpochSeconds();
  result = zoneProcessor.findByEpochSeconds(epochSeconds);
  assertEqual(result.type, FindResult::kTypeOverlap);
  assertEqual(result.fold, 0);
  assertEqual(-8*60, result.stdOffsetMinutes);
  assertEqual(1*60, result.dstOffsetMinutes);
  assertEqual(-8*60, result.reqStdOffsetMinutes);
  assertEqual(1*60, result.reqDstOffsetMinutes);
  assertEqual("PDT", result.abbrev);

  // 01:59 just before fall back, overlap, first occurrence
  dt = OffsetDateTime::forComponents(2018, 11, 4, 1, 59, 0,
      TimeOffset::forHours(-7));
  epochSeconds = dt.toEpochSeconds();
  result = zoneProcessor.findByEpochSeconds(epochSeconds);
  assertEqual(result.type, FindResult::kTypeOverlap);
  assertEqual(result.fold, 0);
  assertEqual(-8*60, result.stdOffsetMinutes);
  assertEqual(1*60, result.dstOffsetMinutes);
  assertEqual(-8*60, result.reqStdOffsetMinutes);
  assertEqual(1*60, result.reqDstOffsetMinutes);
  assertEqual("PDT", result.abbrev);

  // 02:00 falls back to the second 01:00, but 02:00 occurs only once
  dt = OffsetDateTime::forComponents(2018, 11, 4, 2, 0, 0,
      TimeOffset::forHours(-7));
  epochSeconds = dt.toEpochSeconds();
  result = zoneProcessor.findByEpochSeconds(epochSeconds);
  assertEqual(result.type, FindResult::kTypeOverlap);
  assertEqual(result.fold, 1);
  assertEqual(-8*60, result.stdOffsetMinutes);
  assertEqual(0*60, result.dstOffsetMinutes);
  assertEqual(-8*60, result.reqStdOffsetMinutes);
  assertEqual(0*60, result.reqDstOffsetMinutes);
  assertEqual("PST", result.abbrev);

  // 01:59, overlap, second occurence
  dt = OffsetDateTime::forComponents(2018, 11, 4, 1, 59, 0,
      TimeOffset::forHours(-8));
  epochSeconds = dt.toEpochSeconds();
  result = zoneProcessor.findByEpochSeconds(epochSeconds);
  assertEqual(result.type, FindResult::kTypeOverlap);
  assertEqual(result.fold, 1);
  assertEqual(-8*60, result.stdOffsetMinutes);
  assertEqual(0*60, result.dstOffsetMinutes);
  assertEqual(-8*60, result.reqStdOffsetMinutes);
  assertEqual(0*60, result.reqDstOffsetMinutes);
  assertEqual("PST", result.abbrev);

  // 03:00 an hour after fall back, no overlap
  dt = OffsetDateTime::forComponents(2018, 11, 4, 3, 0, 0,
      TimeOffset::forHours(-7));
  epochSeconds = dt.toEpochSeconds();
  result = zoneProcessor.findByEpochSeconds(epochSeconds);
  assertEqual(result.type, FindResult::kTypeExact);
  assertEqual(result.fold, 0);
  assertEqual(-8*60, result.stdOffsetMinutes);
  assertEqual(0*60, result.dstOffsetMinutes);
  assertEqual(-8*60, result.reqStdOffsetMinutes);
  assertEqual(0*60, result.reqDstOffsetMinutes);
  assertEqual("PST", result.abbrev);
}

test(ExtendedZoneProcessorTest, findByEpochSeconds_outOfBounds) {
  EpochYearContext context(2000); // epoch to 2000 temporarily
  ExtendedZoneProcessor zoneProcessor(&kZoneAmerica_Los_Angeles);
  OffsetDateTime dt;
  acetime_t epochSeconds;

  assertEqual(1980, kZoneContext.startYear);
  assertEqual(10000, kZoneContext.untilYear);

  // 1970 > LocalDate::kMinYear so dt is valid, and
  dt = OffsetDateTime::forComponents(1970, 3, 11, 1, 59, 59,
      TimeOffset::forHours(-8));
  assertFalse(dt.isError());
  // 1998 is within roughly 50 years of Epoch::currentEpochYear() of 2050 so
  // toEpochSeconds() still works.
  epochSeconds = dt.toEpochSeconds();
  assertNotEqual(epochSeconds, LocalDate::kInvalidEpochSeconds);
  // But 1998 < ZoneContext.startYear, so FindResult not found.
  FindResult result = zoneProcessor.findByEpochSeconds(epochSeconds);
  assertEqual(result.type, FindResult::kTypeNotFound);

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
  assertEqual(result.type, FindResult::kTypeNotFound);
}

//---------------------------------------------------------------------------

// Test that getOffsetDateTime(const LocalDateTime&) handles fold parameter
// correctly.
test(ExtendedZoneProcessorTest, findByLocalDateTime) {
  ExtendedZoneProcessor zoneProcessor(&kZoneAmerica_Los_Angeles);
  FindResult result;
  LocalDateTime ldt;

  // 01:59, before spring forward
  ldt = LocalDateTime::forComponents(2022, 3, 13, 1, 59, 0, 0 /*fold*/);
  result = zoneProcessor.findByLocalDateTime(ldt);
  assertEqual(result.type, FindResult::kTypeExact);
  assertEqual(result.fold, 0);
  assertEqual(-8*60, result.stdOffsetMinutes);
  assertEqual(0*60, result.dstOffsetMinutes);
  assertEqual(-8*60, result.reqStdOffsetMinutes);
  assertEqual(0*60, result.reqDstOffsetMinutes);
  assertEqual("PST", result.abbrev);

  // 02:00, in gap, fold=0 transition normalizes to 03:00-07:00
  ldt = LocalDateTime::forComponents(2022, 3, 13, 2, 0, 0, 0 /*fold*/);
  result = zoneProcessor.findByLocalDateTime(ldt);
  assertEqual(result.type, FindResult::kTypeGap);
  assertEqual(result.fold, 0);
  assertEqual(-8*60, result.stdOffsetMinutes);
  assertEqual(1*60, result.dstOffsetMinutes);
  assertEqual(-8*60, result.reqStdOffsetMinutes);
  assertEqual(0*60, result.reqDstOffsetMinutes);
  assertEqual("PDT", result.abbrev);

  // 02:00, in gap, fold=1 normalizes to 01:00-08:00
  ldt = LocalDateTime::forComponents(2022, 3, 13, 2, 0, 0, 1 /*fold*/);
  result = zoneProcessor.findByLocalDateTime(ldt);
  assertEqual(result.type, FindResult::kTypeGap);
  assertEqual(result.fold, 0);
  assertEqual(-8*60, result.stdOffsetMinutes);
  assertEqual(0*60, result.dstOffsetMinutes);
  assertEqual(-8*60, result.reqStdOffsetMinutes);
  assertEqual(1*60, result.reqDstOffsetMinutes);
  assertEqual("PST", result.abbrev);

  // 02:29 in gap, fold=0 uses -08:00 to convert to epochSeconds, then
  // normalizes to -07:00.
  ldt = LocalDateTime::forComponents(2022, 3, 13, 2, 29, 0, 0 /*fold*/);
  result = zoneProcessor.findByLocalDateTime(ldt);
  assertEqual(result.type, FindResult::kTypeGap);
  assertEqual(result.fold, 0);
  assertEqual(-8*60, result.stdOffsetMinutes);
  assertEqual(1*60, result.dstOffsetMinutes);
  assertEqual(-8*60, result.reqStdOffsetMinutes);
  assertEqual(0*60, result.reqDstOffsetMinutes);
  assertEqual("PDT", result.abbrev);

  // 02:29 in gap, fold==1 uses -07:00 to convert to epochSeconds, then
  // normalizes to -08:00
  ldt = LocalDateTime::forComponents(2022, 3, 13, 2, 29, 0, 1 /*fold*/);
  result = zoneProcessor.findByLocalDateTime(ldt);
  assertEqual(result.type, FindResult::kTypeGap);
  assertEqual(result.fold, 0);
  assertEqual(-8*60, result.stdOffsetMinutes);
  assertEqual(0*60, result.dstOffsetMinutes);
  assertEqual(-8*60, result.reqStdOffsetMinutes);
  assertEqual(1*60, result.reqDstOffsetMinutes);
  assertEqual("PST", result.abbrev);

  // 00:59, before fall back, before any overlap
  ldt = LocalDateTime::forComponents(2022, 11, 6, 0, 59, 0, 0 /*fold*/);
  result = zoneProcessor.findByLocalDateTime(ldt);
  assertEqual(result.type, FindResult::kTypeExact);
  assertEqual(result.fold, 0);
  assertEqual(-8*60, result.stdOffsetMinutes);
  assertEqual(1*60, result.dstOffsetMinutes);
  assertEqual(-8*60, result.reqStdOffsetMinutes);
  assertEqual(1*60, result.reqDstOffsetMinutes);
  assertEqual("PDT", result.abbrev);

  // 01:00, after fall back, overlap, select first
  ldt = LocalDateTime::forComponents(2022, 11, 6, 1, 0, 0, 0 /*fold*/);
  result = zoneProcessor.findByLocalDateTime(ldt);
  assertEqual(result.type, FindResult::kTypeOverlap);
  assertEqual(result.fold, 0);
  assertEqual(-8*60, result.stdOffsetMinutes);
  assertEqual(1*60, result.dstOffsetMinutes);
  assertEqual(-8*60, result.reqStdOffsetMinutes);
  assertEqual(1*60, result.reqDstOffsetMinutes);
  assertEqual("PDT", result.abbrev);

  // 01:00, overlap, select second
  ldt = LocalDateTime::forComponents(2022, 11, 6, 1, 0, 0, 1 /*fold*/);
  result = zoneProcessor.findByLocalDateTime(ldt);
  assertEqual(result.type, FindResult::kTypeOverlap);
  assertEqual(result.fold, 1);
  assertEqual(-8*60, result.stdOffsetMinutes);
  assertEqual(0*60, result.dstOffsetMinutes);
  assertEqual(-8*60, result.reqStdOffsetMinutes);
  assertEqual(0*60, result.reqDstOffsetMinutes);
  assertEqual("PST", result.abbrev);

  // 01:29, fold=0, before fall back, in overlap, select first
  ldt = LocalDateTime::forComponents(2022, 11, 6, 1, 29, 0, 0 /*fold*/);
  result = zoneProcessor.findByLocalDateTime(ldt);
  assertEqual(result.type, FindResult::kTypeOverlap);
  assertEqual(result.fold, 0);
  assertEqual(-8*60, result.stdOffsetMinutes);
  assertEqual(1*60, result.dstOffsetMinutes);
  assertEqual(-8*60, result.reqStdOffsetMinutes);
  assertEqual(1*60, result.reqDstOffsetMinutes);
  assertEqual("PDT", result.abbrev);

  // 01:29, fold=1, after fall back, in overlap, select second
  ldt = LocalDateTime::forComponents(2022, 11, 6, 1, 29, 0, 1 /*fold*/);
  result = zoneProcessor.findByLocalDateTime(ldt);
  assertEqual(result.fold, 1);
  assertEqual(result.type, FindResult::kTypeOverlap);
  assertEqual(-8*60, result.stdOffsetMinutes);
  assertEqual(0*60, result.dstOffsetMinutes);
  assertEqual(-8*60, result.reqStdOffsetMinutes);
  assertEqual(0*60, result.reqDstOffsetMinutes);
  assertEqual("PST", result.abbrev);

  // 02:00, after fall back, should be no overlap because it occurs only once
  ldt = LocalDateTime::forComponents(2022, 11, 6, 2, 0, 0, 0 /*fold*/);
  result = zoneProcessor.findByLocalDateTime(ldt);
  assertEqual(result.type, FindResult::kTypeExact);
  assertEqual(result.fold, 0);
  assertEqual(-8*60, result.stdOffsetMinutes);
  assertEqual(0*60, result.dstOffsetMinutes);
  assertEqual(-8*60, result.reqStdOffsetMinutes);
  assertEqual(0*60, result.reqDstOffsetMinutes);
  assertEqual("PST", result.abbrev);

  // 02:30, way after fall back, no overlap
  ldt = LocalDateTime::forComponents(2022, 11, 6, 2, 30, 0, 0 /*fold*/);
  result = zoneProcessor.findByLocalDateTime(ldt);
  assertEqual(result.type, FindResult::kTypeExact);
  assertEqual(result.fold, 0);
  assertEqual(-8*60, result.stdOffsetMinutes);
  assertEqual(0*60, result.dstOffsetMinutes);
  assertEqual(-8*60, result.reqStdOffsetMinutes);
  assertEqual(0*60, result.reqDstOffsetMinutes);
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
