#line 2 "ExtendedZoneSpecifierTest.ino"

#include <AUnit.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time;
using namespace ace_time::extended;

// --------------------------------------------------------------------------
// ExtendedZoneSpecifier
// --------------------------------------------------------------------------

test(ExtendedZoneSpecifierTest, compareEraToYearMonth) {
  common::ZoneEra era = {0, nullptr, 0, "", 0, 1, 2, 12, 'w'};
  assertEqual(1, ExtendedZoneSpecifier::compareEraToYearMonth(&era, 0, 1));
  assertEqual(1, ExtendedZoneSpecifier::compareEraToYearMonth(&era, 0, 1));
  assertEqual(-1, ExtendedZoneSpecifier::compareEraToYearMonth(&era, 0, 2));
  assertEqual(-1, ExtendedZoneSpecifier::compareEraToYearMonth(&era, 0, 3));

  common::ZoneEra era2 = {0, nullptr, 0, "", 0, 1, 0, 0, 'w'};
  assertEqual(0, ExtendedZoneSpecifier::compareEraToYearMonth(&era2, 0, 1));
}

test(ExtendedZoneSpecifierTest, createMatch) {
  // UNTIL = 2000-01-02 3:00
  common::ZoneEra prev = {0, nullptr, 0, "", 0, 1, 2, 3, 'w'};
  // UNTIL = 2002-03-04 5:00
  common::ZoneEra era = {0, nullptr, 0, "", 2, 3, 4, 5, 'w'};

  extended::YearMonthTuple startYm = {0, 12};
  extended::YearMonthTuple untilYm = {1, 2};
  extended::ZoneMatch match = ExtendedZoneSpecifier::createMatch(
      &prev, &era, startYm, untilYm);
  assertTrue((match.startDateTime == extended::DateTuple{0, 12, 1, 0, 'w'}));
  assertTrue((match.untilDateTime == extended::DateTuple{1, 2, 1, 0, 'w'}));
  assertTrue(&era == match.era);

  startYm = {-1, 12};
  untilYm = {3, 2};
  match = ExtendedZoneSpecifier::createMatch(&prev, &era, startYm, untilYm);
  assertTrue((match.startDateTime == extended::DateTuple{0, 1, 2, 3, 'w'}));
  assertTrue((match.untilDateTime == extended::DateTuple{2, 3, 4, 5, 'w'}));
  assertTrue(&era == match.era);
}

test(ExtendedZoneSpecifierTest, findMatches) {
  // TODO: Implement
}

test(ExtendedZoneSpecifierTest, getTransitionTime) {
  // TODO: Implement
}

test(ExtendedZoneSpecifierTest, createTransitionForYear) {
  // TODO: Implement
}

test(ExtendedZoneSpecifierTest, normalizeDateTuple) {
  DateTuple dtp;

  dtp = {0, 1, 1, 0, 'w'};
  ExtendedZoneSpecifier::normalizeDateTuple(&dtp);
  assertTrue((dtp == DateTuple{0, 1, 1, 0, 'w'}));

  dtp = {0, 1, 1, 95, 'w'};
  ExtendedZoneSpecifier::normalizeDateTuple(&dtp);
  assertTrue((dtp == DateTuple{0, 1, 1, 95, 'w'}));

  dtp = {0, 1, 1, 96, 'w'};
  ExtendedZoneSpecifier::normalizeDateTuple(&dtp);
  assertTrue((dtp == DateTuple{0, 1, 2, 0, 'w'}));

  dtp = {0, 1, 1, 97, 'w'};
  ExtendedZoneSpecifier::normalizeDateTuple(&dtp);
  assertTrue((dtp == DateTuple{0, 1, 2, 1, 'w'}));

  dtp = {0, 1, 1, -96, 'w'};
  ExtendedZoneSpecifier::normalizeDateTuple(&dtp);
  assertTrue((dtp == DateTuple{-01, 12, 31, 0, 'w'}));

  dtp = {0, 1, 1, -97, 'w'};
  ExtendedZoneSpecifier::normalizeDateTuple(&dtp);
  assertTrue((dtp == DateTuple{-01, 12, 31, -1, 'w'}));
}

test(ExtendedZoneSpecifierTest, expandDateTuple) {
  DateTuple tt;
  DateTuple tts;
  DateTuple ttu;
  int8_t offsetCode = 8;
  int8_t deltaCode = 4;

  tt = {0, 1, 30, 12, 'w'};
  ExtendedZoneSpecifier::expandDateTuple(&tt, &tts, &ttu,
      offsetCode, deltaCode);
  assertTrue((tt == DateTuple{0, 1, 30, 12, 'w'}));
  assertTrue((tts == DateTuple{0, 1, 30, 8, 's'}));
  assertTrue((ttu == DateTuple{0, 1, 30, 0, 'u'}));

  tt = {0, 1, 30, 8, 's'};
  ExtendedZoneSpecifier::expandDateTuple(&tt, &tts, &ttu,
      offsetCode, deltaCode);
  assertTrue((tt == DateTuple{0, 1, 30, 12, 'w'}));
  assertTrue((tts == DateTuple{0, 1, 30, 8, 's'}));
  assertTrue((ttu == DateTuple{0, 1, 30, 0, 'u'}));

  tt = {0, 1, 30, 0, 'u'};
  ExtendedZoneSpecifier::expandDateTuple(&tt, &tts, &ttu,
      offsetCode, deltaCode);
  assertTrue((tt == DateTuple{0, 1, 30, 12, 'w'}));
  assertTrue((tts == DateTuple{0, 1, 30, 8, 's'}));
  assertTrue((ttu == DateTuple{0, 1, 30, 0, 'u'}));
}

test(ExtendedZoneSpecifierTest, calcInteriorYears) {
  const uint8_t kMaxInteriorYears = 4;
  int8_t interiorYears[kMaxInteriorYears];

  uint8_t num = ExtendedZoneSpecifier::calcInteriorYears(
      interiorYears, kMaxInteriorYears, -2, -1, 0, 2);
  assertEqual(0, num);

  num = ExtendedZoneSpecifier::calcInteriorYears(
      interiorYears, kMaxInteriorYears, 3, 5, 0, 2);
  assertEqual(0, num);

  num = ExtendedZoneSpecifier::calcInteriorYears(
      interiorYears, kMaxInteriorYears, -2, 0, 0, 2);
  assertEqual(1, num);
  assertEqual(0, interiorYears[0]);

  num = ExtendedZoneSpecifier::calcInteriorYears(
      interiorYears, kMaxInteriorYears, 2, 4, 0, 2);
  assertEqual(1, num);
  assertEqual(2, interiorYears[0]);

  num = ExtendedZoneSpecifier::calcInteriorYears(
      interiorYears, kMaxInteriorYears, 1, 2, 0, 2);
  assertEqual(2, num);
  assertEqual(1, interiorYears[0]);
  assertEqual(2, interiorYears[1]);

  num = ExtendedZoneSpecifier::calcInteriorYears(
      interiorYears, kMaxInteriorYears, -1, 3, 0, 2);
  assertEqual(3, num);
  assertEqual(0, interiorYears[0]);
  assertEqual(1, interiorYears[1]);
  assertEqual(2, interiorYears[2]);
}

test(ExtendedZoneSpecifierTest, getMostRecentPriorYear) {
  int8_t yearTiny;

  yearTiny = ExtendedZoneSpecifier::getMostRecentPriorYear(-2, -1, 0, 2);
  assertEqual(-1, yearTiny);

  yearTiny = ExtendedZoneSpecifier::getMostRecentPriorYear(3, 5, 0, 2);
  assertEqual(LocalDate::kInvalidYearTiny, yearTiny);

  yearTiny = ExtendedZoneSpecifier::getMostRecentPriorYear(-2, 0, 0, 2);
  assertEqual(-1, yearTiny);

  yearTiny = ExtendedZoneSpecifier::getMostRecentPriorYear(2, 4, 0, 2);
  assertEqual(LocalDate::kInvalidYearTiny, yearTiny);

  yearTiny = ExtendedZoneSpecifier::getMostRecentPriorYear(1, 2, 0, 2);
  assertEqual(LocalDate::kInvalidYearTiny, yearTiny);

  yearTiny = ExtendedZoneSpecifier::getMostRecentPriorYear(-1, 3, 0, 2);
  assertEqual(-1, yearTiny);
}

test(ExtendedZoneSpecifierTest, compareTransitionToMatchFuzzy) {
  const ZoneMatch match = {
    {0, 1, 1, 0, 'w'} /* startDateTime */,
    {1, 1, 1, 0, 'w'} /* untilDateTime */,
    nullptr
  };

  extended::Transition transition = {
    &match /*match*/, nullptr /*rule*/, {-1, 11, 1, 0, 'w'} /*transitionTime*/,
    {}, {}, {}, {0}, 0, false
  };
  assertEqual(-1, ExtendedZoneSpecifier::compareTransitionToMatchFuzzy(
      &transition, &match));

  transition = {
    &match /*match*/, nullptr /*rule*/, {-1, 12, 1, 0, 'w'} /*transitionTime*/,
    {}, {}, {}, {0}, 0, false
  };
  assertEqual(1, ExtendedZoneSpecifier::compareTransitionToMatchFuzzy(
      &transition, &match));

  transition = {
    &match /*match*/, nullptr /*rule*/, {0, 1, 1, 0, 'w'} /*transitionTime*/,
    {}, {}, {}, {0}, 0, false
  };
  assertEqual(1, ExtendedZoneSpecifier::compareTransitionToMatchFuzzy(
      &transition, &match));

  transition = {
    &match /*match*/, nullptr /*rule*/, {1, 1, 1, 0, 'w'} /*transitionTime*/,
    {}, {}, {}, {0}, 0, false
  };
  assertEqual(1, ExtendedZoneSpecifier::compareTransitionToMatchFuzzy(
      &transition, &match));

  transition = {
    &match /*match*/, nullptr /*rule*/, {1, 2, 1, 0, 'w'} /*transitionTime*/,
    {}, {}, {}, {0}, 0, false
  };
  assertEqual(1, ExtendedZoneSpecifier::compareTransitionToMatchFuzzy(
      &transition, &match));

  transition = {
    &match /*match*/, nullptr /*rule*/, {1, 3, 1, 0, 'w'} /*transitionTime*/,
    {}, {}, {}, {0}, 0, false
  };
  assertEqual(2, ExtendedZoneSpecifier::compareTransitionToMatchFuzzy(
      &transition, &match));
}


test(ExtendedZoneSpecifierTest, compareTransitionToMatch) {
  const ZoneMatch match = {
    {0, 1, 1, 0, 'w'} /*startDateTime*/,
    {1, 1, 1, 0, 'w'} /*untilDateTime*/,
    nullptr /*era*/
  };

  extended::Transition transition = {
    &match /*match*/, nullptr /*rule*/, {-1, 12, 31, 0, 'w'} /*transitionTime*/,
    {}, {}, {}, {0}, 0, false
  };
  assertEqual(-1, ExtendedZoneSpecifier::compareTransitionToMatch(
      &transition, &match));

  transition = {
    &match /*match*/, nullptr /*rule*/, {0, 1, 1, 0, 'w'} /*transitionTime*/,
    {}, {}, {}, {0}, 0, false
  };
  assertEqual(0, ExtendedZoneSpecifier::compareTransitionToMatch(
      &transition, &match));

  transition = {
    &match /*match*/, nullptr /*rule*/, {0, 1, 2, 0, 'w'} /*transitionTime*/,
    {}, {}, {}, {0}, 0, false
  };
  assertEqual(1, ExtendedZoneSpecifier::compareTransitionToMatch(
      &transition, &match));

  transition = {
    &match /*match*/, nullptr /*rule*/, {1, 1, 2, 0, 'w'} /*transitionTime*/,
    {}, {}, {}, {0}, 0, false
  };
  assertEqual(2, ExtendedZoneSpecifier::compareTransitionToMatch(
      &transition, &match));
}

test(ExtendedZoneSpecifierTest, processActiveTransition) {
  extended::Transition* prior = nullptr;
  const ZoneMatch match = {
    {0, 1, 1, 0, 'w'} /*startDateTime*/,
    {1, 1, 1, 0, 'w'} /*untilDateTime*/,
    nullptr /*era*/
  };

  // This transition occurs before the match, so prior should be filled.
  extended::Transition transition0 = {
    &match /*match*/, nullptr /*rule*/, {-1, 12, 31, 0, 'w'} /*transitionTime*/,
    {}, {}, {}, {0}, 0, false /* active */
  };
  ExtendedZoneSpecifier::processActiveTransition(&match, &transition0, &prior);
  assertTrue(transition0.active);
  assertTrue(prior == &transition0);

  // This occurs at exactly match.startDateTime, so should replace
  extended::Transition transition1 = {
    &match /*match*/, nullptr /*rule*/, {0, 1, 1, 0, 'w'} /*transitionTime*/,
    {}, {}, {}, {0}, 0, false
  };
  ExtendedZoneSpecifier::processActiveTransition(&match, &transition1, &prior);
  assertTrue(transition1.active);
  assertTrue(prior == &transition1);

  // An interior transition. Prior should not change.
  extended::Transition transition2 = {
    &match /*match*/, nullptr /*rule*/, {0, 1, 2, 0, 'w'} /*transitionTime*/,
    {}, {}, {}, {0}, 0, false
  };
  ExtendedZoneSpecifier::processActiveTransition(&match, &transition2, &prior);
  assertTrue(transition2.active);
  assertTrue(prior == &transition1);
  
  // Occurs after match.untilDateTime, so should be rejected.
  extended::Transition transition3 = {
    &match /*match*/, nullptr /*rule*/, {1, 1, 2, 0, 'w'} /*transitionTime*/,
    {}, {}, {}, {0}, 0, false
  };
  assertFalse(transition3.active);
  assertTrue(prior == &transition1);
}

test(ExtendedZoneSpecifierTest, fixTransitionTimes) {
  // TODO: Implement
}

test(ExtendedZoneSpecifierTest, generateStartUntilTimes) {
  // Create simplified ZoneEras which approximate America/Los_Angeles
  static common::ZoneEra kEra1 = {
    -32 /*offsetCode*/,
    nullptr,
    0 /*deltaCode*/,
    "PST" /*format*/,
    19 /*untilYearTiny*/,
    3 /*untilMonth*/,
    10 /*untilDay*/,
    2*4 /*untilTimeCode*/,
    'w' /*untilTimeModifier*/
  };
  static common::ZoneEra kEra2 = {
    -32 /*offsetCode*/,
    nullptr,
    4 /*deltaCode*/,
    "PDT" /*format*/,
    19 /*untilYearTiny*/,
    11 /*untilMonth*/,
    3 /*untilDay*/,
    2*4 /*untilTimeCode*/,
    'w' /*untilTimeModifier*/
  };
  static common::ZoneEra kEra3 = {
    -32 /*offsetCode*/,
    nullptr,
    0 /*deltaCode*/,
    "PST" /*format*/,
    20 /*untilYearTiny*/,
    3 /*untilMonth*/,
    8 /*untilDay*/,
    2*4 /*untilTimeCode*/,
    'w' /*untilTimeModifier*/
  };
  extended::YearMonthTuple startYm = {18, 12};
  extended::YearMonthTuple untilYm = {20, 2};

  // Create 3 ZoneMatches that follow from the ZoneEras
  extended::ZoneMatch match1 = ExtendedZoneSpecifier::createMatch(
      &ExtendedZoneSpecifier::kAnchorEra, &kEra1, startYm, untilYm);
  assertTrue((match1.startDateTime == extended::DateTuple{18, 12, 1, 0, 'w'}));
  assertTrue((match1.untilDateTime == extended::DateTuple{19, 3, 10, 8, 'w'}));
  assertTrue(&kEra1 == match1.era);

  extended::ZoneMatch match2 = ExtendedZoneSpecifier::createMatch(
      &kEra1, &kEra2, startYm, untilYm);
  assertTrue((match2.startDateTime == extended::DateTuple{19, 3, 10, 8, 'w'}));
  assertTrue((match2.untilDateTime == extended::DateTuple{19, 11, 3, 8, 'w'}));
  assertTrue(&kEra2 == match2.era);

  extended::ZoneMatch match3 = ExtendedZoneSpecifier::createMatch(
      &kEra2, &kEra3, startYm, untilYm);
  assertTrue((match3.startDateTime == extended::DateTuple{19, 11, 3, 8, 'w'}));
  assertTrue((match3.untilDateTime == extended::DateTuple{20, 2, 1, 0, 'w'}));
  assertTrue(&kEra3 == match3.era);

  TransitionStorage<4> storage;
  storage.init();

  // Create 3 Transitions corresponding to the matches.
  // Implements ExtendedZoneSpecifier::findTransitionsFromSimpleMatch().
  extended::Transition* transition1 = storage.getFreeAgent();
  transition1->match = &match1;
  transition1->rule = nullptr;
  transition1->transitionTime = match1.startDateTime;
  transition1->active = true;
  storage.addFreeAgentToCandidatePool();

  extended::Transition* transition2 = storage.getFreeAgent();
  transition2->match = &match2;
  transition2->rule = nullptr;
  transition2->transitionTime = match2.startDateTime;
  transition2->active = true;
  storage.addFreeAgentToCandidatePool();

  extended::Transition* transition3 = storage.getFreeAgent();
  transition3->match = &match3;
  transition3->rule = nullptr;
  transition3->transitionTime = match3.startDateTime;
  transition3->active = true;
  storage.addFreeAgentToCandidatePool();

  // Move actives to Active pool.
  storage.addActiveCandidatesToActivePool();
  extended::Transition** begin = storage.getActivePoolBegin();
  extended::Transition** end = storage.getActivePoolEnd();
  assertEqual(3, (int) (end - begin));
  assertTrue(begin[0] == transition1);
  assertTrue(begin[1] == transition2);
  assertTrue(begin[2] == transition3);

  // Fix the transition times, expanding to 's' and 'u'
  ExtendedZoneSpecifier::fixTransitionTimes(begin, end);

  // Verify. The first Transition is extended to -infinity.
  assertTrue((transition1->transitionTime
      == extended::DateTuple{18, 12, 1, 0, 'w'}));
  assertTrue((transition1->transitionTimeS
      == extended::DateTuple{18, 12, 1, 0, 's'}));
  assertTrue((transition1->transitionTimeU
      == extended::DateTuple{18, 12, 1, 32, 'u'}));

  // Second transition uses the UTC offset of the first.
  assertTrue((transition2->transitionTime
      == extended::DateTuple{19, 3, 10, 8, 'w'}));
  assertTrue((transition2->transitionTimeS
      == extended::DateTuple{19, 3, 10, 8, 's'}));
  assertTrue((transition2->transitionTimeU
      == extended::DateTuple{19, 3, 10, 40, 'u'}));
}

test(ExtendedZoneSpecifierTest, calcAbbreviations) {
  // TODO: Implement
}

// --------------------------------------------------------------------------
// TransitionStorage
// --------------------------------------------------------------------------

test(TransitionStorageTest, getFreeAgent) {
  TransitionStorage<4> storage;
  storage.init();

  Transition* freeAgent = storage.getFreeAgent();
  assertTrue(freeAgent == &storage.mPool[0]);
  storage.addFreeAgentToActivePool();

  freeAgent = storage.getFreeAgent();
  assertTrue(freeAgent == &storage.mPool[1]);
  storage.addFreeAgentToActivePool();

  freeAgent = storage.getFreeAgent();
  assertTrue(freeAgent == &storage.mPool[2]);
  storage.addFreeAgentToActivePool();

  freeAgent = storage.getFreeAgent();
  assertTrue(freeAgent == &storage.mPool[3]);
  storage.addFreeAgentToActivePool();

  // Verify overflow checking.
  freeAgent = storage.getFreeAgent();
  assertTrue(freeAgent == &storage.mPool[3]);
}

test(TransitionStorageTest, getFreeAgent2) {
  TransitionStorage<4> storage;
  storage.init();

  Transition* freeAgent = storage.getFreeAgent();
  assertTrue(freeAgent == storage.mTransitions[0]);
  storage.addFreeAgentToCandidatePool();
  assertEqual(1, storage.mIndexFree);

  freeAgent = storage.getFreeAgent();
  assertTrue(freeAgent == storage.mTransitions[1]);
  storage.addFreeAgentToCandidatePool();
  assertEqual(2, storage.mIndexFree);

  freeAgent = storage.getFreeAgent();
  assertTrue(freeAgent == storage.mTransitions[2]);
  storage.addFreeAgentToCandidatePool();
  assertEqual(3, storage.mIndexFree);

  freeAgent = storage.getFreeAgent();
  assertTrue(freeAgent == storage.mTransitions[3]);
  storage.addFreeAgentToCandidatePool();
  assertEqual(4, storage.mIndexFree);

  // Verify overflow checking.
  freeAgent = storage.getFreeAgent();
  assertTrue(freeAgent == storage.mTransitions[3]);
}

test(TransitionStorageTest, addFreeAgentToActivePool) {
  TransitionStorage<4> storage;
  storage.init();

  Transition* freeAgent = storage.getFreeAgent();
  assertTrue(freeAgent == &storage.mPool[0]);

  storage.addFreeAgentToActivePool();
  assertEqual(1, storage.mIndexPrior);
  assertEqual(1, storage.mIndexCandidates);
  assertEqual(1, storage.mIndexFree);
}

test(TransitionStorageTest, reservePrior) {
  TransitionStorage<4> storage;
  storage.init();
  Transition* prior = storage.reservePrior();
  assertTrue(prior == &storage.mPool[0]);
  assertEqual(0, storage.mIndexPrior);
  assertEqual(1, storage.mIndexCandidates);
  assertEqual(1, storage.mIndexFree);

  storage.addPriorToCandidatePool();
  assertEqual(0, storage.mIndexPrior);
  assertEqual(0, storage.mIndexCandidates);
  assertEqual(1, storage.mIndexFree);
}

test(TransitionStorageTest, setFreeAgentAsPrior) {
  TransitionStorage<4> storage;
  storage.init();

  Transition* prior = storage.reservePrior();
  prior->active = false;
  Transition* freeAgent = storage.getFreeAgent();
  freeAgent->active = true;
  storage.setFreeAgentAsPrior();

  // Verify that the two have been swapped.
  prior = storage.getPrior();
  freeAgent = storage.getFreeAgent();
  assertTrue(prior->active);
  assertFalse(freeAgent->active);
}

test(TransitionStorageTest, addFreeAgentToCandidatePool) {
  TransitionStorage<4> storage;
  storage.init();

  // create Prior to make it interesting
  /*Transition* prior =*/ storage.reservePrior();

  // Verify that addFreeAgentToCandidatePool() does not touch prior transition
  Transition* freeAgent = storage.getFreeAgent();
  freeAgent->transitionTime = {0, 1, 2, 3, 'w'};
  storage.addFreeAgentToCandidatePool();
  assertEqual(0, storage.mIndexPrior);
  assertEqual(1, storage.mIndexCandidates);
  assertEqual(2, storage.mIndexFree);

  freeAgent = storage.getFreeAgent();
  freeAgent->transitionTime = {2, 3, 4, 5, 'w'};
  storage.addFreeAgentToCandidatePool();
  assertEqual(0, storage.mIndexPrior);
  assertEqual(1, storage.mIndexCandidates);
  assertEqual(3, storage.mIndexFree);

  freeAgent = storage.getFreeAgent();
  freeAgent->transitionTime = {1, 2, 3, 4, 'w'};
  storage.addFreeAgentToCandidatePool();
  assertEqual(0, storage.mIndexPrior);
  assertEqual(1, storage.mIndexCandidates);
  assertEqual(4, storage.mIndexFree);

  // Assert that the transitions are sorted
  assertEqual(0, storage.getTransition(1)->transitionTime.yearTiny);
  assertEqual(1, storage.getTransition(2)->transitionTime.yearTiny);
  assertEqual(2, storage.getTransition(3)->transitionTime.yearTiny);
}

test(TransitionStorageTest, addActiveCandidatesToActivePool) {
  TransitionStorage<4> storage;
  storage.init();

  // create Prior to make it interesting
  Transition* prior = storage.reservePrior();
  prior->transitionTime = {-1, 0, 1, 2, 'w'};
  prior->active = true;

  // Add 3 transitions to Candidate pool, 2 active, 1 inactive.
  Transition* freeAgent = storage.getFreeAgent();
  freeAgent->transitionTime = {0, 1, 2, 3, 'w'};
  freeAgent->active = true;
  storage.addFreeAgentToCandidatePool();

  freeAgent = storage.getFreeAgent();
  freeAgent->transitionTime = {2, 3, 4, 5, 'w'};
  freeAgent->active = true;
  storage.addFreeAgentToCandidatePool();

  freeAgent = storage.getFreeAgent();
  freeAgent->transitionTime = {1, 2, 3, 4, 'w'};
  freeAgent->active = false;
  storage.addFreeAgentToCandidatePool();

  // Add prior into the Candidate pool.
  storage.addPriorToCandidatePool();

  // Add the actives to the Active pool.
  storage.addActiveCandidatesToActivePool();

  // Verify that there are 3 transitions in the Active pool.
  assertEqual(3, storage.mIndexPrior);
  assertEqual(3, storage.mIndexCandidates);
  assertEqual(3, storage.mIndexFree);
  assertEqual(-1, storage.getTransition(0)->transitionTime.yearTiny);
  assertEqual(0, storage.getTransition(1)->transitionTime.yearTiny);
  assertEqual(2, storage.getTransition(2)->transitionTime.yearTiny);
}

test(TransitionStorageTest, findTransition) {
  TransitionStorage<4> storage;
  storage.init();

  // Add 3 transitions to Candidate pool, 2 active, 1 inactive.
  Transition* freeAgent = storage.getFreeAgent();
  freeAgent->transitionTime = {0, 1, 2, 3, 'w'};
  freeAgent->active = true;
  freeAgent->startEpochSeconds = 0;
  storage.addFreeAgentToCandidatePool();

  freeAgent = storage.getFreeAgent();
  freeAgent->transitionTime = {1, 2, 3, 4, 'w'};
  freeAgent->active = true;
  freeAgent->startEpochSeconds = 10;
  storage.addFreeAgentToCandidatePool();

  freeAgent = storage.getFreeAgent();
  freeAgent->transitionTime = {2, 3, 4, 5, 'w'};
  freeAgent->active = true;
  freeAgent->startEpochSeconds = 20;
  storage.addFreeAgentToCandidatePool();

  // Add the actives to the Active pool.
  storage.addActiveCandidatesToActivePool();

  // Check that we can find the transitions using the startEpochSeconds.

  const Transition* t = storage.findTransition(1);
  assertEqual(0, t->transitionTime.yearTiny);

  t = storage.findTransition(9);
  assertEqual(0, t->transitionTime.yearTiny);

  t = storage.findTransition(10);
  assertEqual(1, t->transitionTime.yearTiny);

  t = storage.findTransition(21);
  assertEqual(2, t->transitionTime.yearTiny);
}

test(TransitionStorageTest, resetCandidatePool) {
  TransitionStorage<4> storage;
  storage.init();

  // Add 2 transitions to Candidate pool, 2 active, 1 inactive.
  Transition* freeAgent = storage.getFreeAgent();
  freeAgent->transitionTime = {0, 1, 2, 3, 'w'};
  freeAgent->active = true;
  storage.addFreeAgentToCandidatePool();
  assertEqual(0, storage.mIndexPrior);
  assertEqual(0, storage.mIndexCandidates);
  assertEqual(1, storage.mIndexFree);

  freeAgent = storage.getFreeAgent();
  freeAgent->transitionTime = {2, 3, 4, 5, 'w'};
  freeAgent->active = true;
  storage.addFreeAgentToCandidatePool();
  assertEqual(0, storage.mIndexPrior);
  assertEqual(0, storage.mIndexCandidates);
  assertEqual(2, storage.mIndexFree);

  // Add active candidates to Active pool. Looks like this
  // already does a resetCandidatePool() effectively.
  storage.addActiveCandidatesToActivePool();
  assertEqual(2, storage.mIndexPrior);
  assertEqual(2, storage.mIndexCandidates);
  assertEqual(2, storage.mIndexFree);

  // This should be a no-op.
  storage.resetCandidatePool();
  assertEqual(2, storage.mIndexPrior);
  assertEqual(2, storage.mIndexCandidates);
  assertEqual(2, storage.mIndexFree);

  freeAgent = storage.getFreeAgent();
  freeAgent->transitionTime = {1, 2, 3, 4, 'w'};
  freeAgent->active = false;
  storage.addFreeAgentToCandidatePool();
  assertEqual(2, storage.mIndexPrior);
  assertEqual(2, storage.mIndexCandidates);
  assertEqual(3, storage.mIndexFree);

  // Reset should remove any remaining candidate transitions.
  storage.resetCandidatePool();
  assertEqual(2, storage.mIndexPrior);
  assertEqual(2, storage.mIndexCandidates);
  assertEqual(2, storage.mIndexFree);
}
// --------------------------------------------------------------------------

void setup() {
#if defined(ARDUINO)
  delay(1000); // wait for stability on some boards to prevent garbage Serial
#endif
  Serial.begin(115200); // ESP8266 default of 74880 not supported on Linux
  while(!Serial); // for the Arduino Leonardo/Micro only
}

void loop() {
  TestRunner::run();
}
