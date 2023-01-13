#line 2 "TransitionStorageTest.ino"

/*
 * Unit tests for Transition and TransitionStorage.
 */

#include <AUnit.h>
#include <AceTime.h>

using namespace ace_time;
using namespace ace_time::extended;
using ace_time::internal::ZoneContext;

//---------------------------------------------------------------------------
// DateTuple.
//---------------------------------------------------------------------------

test(TransitionStorage, dateTupleOperatorLessThan) {
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

test(TransitionStorage, dateTupleOperatorEquals) {
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

test(TransitionStorage, normalizeDateTuple) {
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

test(TransitionStorage, substractDateTuple) {
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

test(TransitionStorageTest, compareDateTupleFuzzy) {
  using ace_time::extended::MatchStatus;
  using ace_time::extended::DateTuple;

  assertEqual(
    (uint8_t) MatchStatus::kPrior,
    (uint8_t) compareDateTupleFuzzy(
      DateTuple{2000, 10, 1, 1, 0},
      DateTuple{2000, 12, 1, 1, 0},
      DateTuple{2002, 2, 1, 1, 0}));

  assertEqual(
    (uint8_t) MatchStatus::kWithinMatch,
    (uint8_t) compareDateTupleFuzzy(
      DateTuple{2000, 11, 1, 1, 0},
      DateTuple{2000, 12, 1, 1, 0},
      DateTuple{2002, 2, 1, 1, 0}));

  assertEqual(
    (uint8_t) MatchStatus::kWithinMatch,
    (uint8_t) compareDateTupleFuzzy(
      DateTuple{2000, 12, 1, 1, 0},
      DateTuple{2000, 12, 1, 1, 0},
      DateTuple{2002, 2, 1, 1, 0}));

  assertEqual(
    (uint8_t) MatchStatus::kWithinMatch,
    (uint8_t) compareDateTupleFuzzy(
      DateTuple{2002, 2, 1, 1, 0},
      DateTuple{2000, 12, 1, 1, 0},
      DateTuple{2002, 2, 1, 1, 0}));

  assertEqual(
    (uint8_t) MatchStatus::kWithinMatch,
    (uint8_t) compareDateTupleFuzzy(
      DateTuple{2002, 3, 1, 1, 0},
      DateTuple{2000, 12, 1, 1, 0},
      DateTuple{2002, 2, 1, 1, 0}));

  assertEqual(
    (uint8_t) MatchStatus::kFarFuture,
    (uint8_t) compareDateTupleFuzzy(
      DateTuple{2002, 4, 1, 1, 0},
      DateTuple{2000, 12, 1, 1, 0},
      DateTuple{2002, 2, 1, 1, 0}));

  // Verify dates whose delta months is greater than 32767. In
  // other words, delta years is greater than 2730.
  assertEqual(
    (uint8_t) MatchStatus::kFarFuture,
    (uint8_t) compareDateTupleFuzzy(
      DateTuple{5000, 4, 1, 1, 0},
      DateTuple{2000, 12, 1, 1, 0},
      DateTuple{2002, 2, 1, 1, 0}));
  assertEqual(
    (uint8_t) MatchStatus::kPrior,
    (uint8_t) compareDateTupleFuzzy(
      DateTuple{1000, 4, 1, 1, 0},
      DateTuple{4000, 12, 1, 1, 0},
      DateTuple{4002, 2, 1, 1, 0}));
}

//---------------------------------------------------------------------------

// Create a custom template instantiation to use a different SIZE than the
// pre-defined typedef in ExtendedZoneProcess::TransitionStorage.
typedef TransitionStorageTemplate<
    4 /*SIZE*/,
    extended::ZoneEraBroker,
    extended::ZonePolicyBroker,
    extended::ZoneRuleBroker> TransitionStorage;

using Transition = TransitionStorage::Transition;
using TransitionForDateTime = TransitionStorage::TransitionForDateTime;

test(TransitionStorageTest, getFreeAgent) {
  TransitionStorage storage;
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
  TransitionStorage storage;
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
  TransitionStorage storage;
  storage.init();

  Transition* freeAgent = storage.getFreeAgent();
  assertTrue(freeAgent == &storage.mPool[0]);

  storage.addFreeAgentToActivePool();
  assertEqual(1, storage.mIndexPrior);
  assertEqual(1, storage.mIndexCandidates);
  assertEqual(1, storage.mIndexFree);
}

test(TransitionStorageTest, reservePrior) {
  TransitionStorage storage;
  storage.init();
  Transition** prior = storage.reservePrior();
  assertTrue(prior == &storage.mTransitions[0]);
  assertEqual(0, storage.mIndexPrior);
  assertEqual(1, storage.mIndexCandidates);
  assertEqual(1, storage.mIndexFree);
}

test(TransitionStorageTest, addPriorToCandidatePool) {
  TransitionStorage storage;
  storage.init();
  Transition** prior = storage.reservePrior();
  assertTrue(prior == &storage.mTransitions[0]);
  assertEqual(0, storage.mIndexPrior);
  assertEqual(1, storage.mIndexCandidates);
  assertEqual(1, storage.mIndexFree);

  storage.addPriorToCandidatePool();
  assertEqual(0, storage.mIndexPrior);
  assertEqual(0, storage.mIndexCandidates);
  assertEqual(1, storage.mIndexFree);
}

test(TransitionStorageTest, setFreeAgentAsPriorIfValid) {
  TransitionStorage storage;
  storage.init();

  // Initial prior
  Transition** priorReservation = storage.reservePrior();
  (*priorReservation)->isValidPrior = false;
  (*priorReservation)->transitionTime = {2002, 3, 4, 5, ZoneContext::kSuffixW};

  // Candiate prior.
  Transition* freeAgent = storage.getFreeAgent();
  freeAgent->isValidPrior = true;
  freeAgent->transitionTime = {2002, 3, 4, 0, ZoneContext::kSuffixW};

  // Should swap because prior->isValidPrior is false.
  storage.setFreeAgentAsPriorIfValid();

  // Verify that the two have been swapped.
  Transition* prior = storage.getPrior();
  freeAgent = storage.getFreeAgent();
  assertTrue(prior->isValidPrior);
  assertFalse(freeAgent->isValidPrior);
  assertTrue(prior->transitionTime
      == DateTuple(2002, 3, 4, 0, ZoneContext::kSuffixW));
  assertTrue(freeAgent->transitionTime
      == DateTuple(2002, 3, 4, 5, ZoneContext::kSuffixW));

  // Another Candidate prior.
  freeAgent = storage.getFreeAgent();
  freeAgent->isValidPrior = true;
  freeAgent->transitionTime = {2002, 3, 4, 6, ZoneContext::kSuffixW};

  // Should swap because the transitionTime is newer
  storage.setFreeAgentAsPriorIfValid();

  // Verify that the two have been swapped.
  prior = storage.getPrior();
  freeAgent = storage.getFreeAgent();
  assertTrue(prior->isValidPrior);
  assertFalse(freeAgent->isValidPrior);
  assertTrue(prior->transitionTime
      == DateTuple(2002, 3, 4, 6, ZoneContext::kSuffixW));
  assertTrue(freeAgent->transitionTime
      == DateTuple(2002, 3, 4, 0, ZoneContext::kSuffixW));
}

test(TransitionStorageTest, addFreeAgentToCandidatePool) {
  TransitionStorage storage;
  storage.init();

  // create Prior to make it interesting
  /*Transition* prior =*/ storage.reservePrior();

  // Verify that addFreeAgentToCandidatePool() does not touch prior transition
  Transition* freeAgent = storage.getFreeAgent();
  freeAgent->transitionTime = {2000, 1, 2, 3, ZoneContext::kSuffixW};
  storage.addFreeAgentToCandidatePool();
  assertEqual(0, storage.mIndexPrior);
  assertEqual(1, storage.mIndexCandidates);
  assertEqual(2, storage.mIndexFree);

  freeAgent = storage.getFreeAgent();
  freeAgent->transitionTime = {2002, 3, 4, 5, ZoneContext::kSuffixW};
  storage.addFreeAgentToCandidatePool();
  assertEqual(0, storage.mIndexPrior);
  assertEqual(1, storage.mIndexCandidates);
  assertEqual(3, storage.mIndexFree);

  freeAgent = storage.getFreeAgent();
  freeAgent->transitionTime = {2001, 2, 3, 4, ZoneContext::kSuffixW};
  storage.addFreeAgentToCandidatePool();
  assertEqual(0, storage.mIndexPrior);
  assertEqual(1, storage.mIndexCandidates);
  assertEqual(4, storage.mIndexFree);

  // Assert that the transitions are sorted
  assertEqual(2000, storage.getTransition(1)->transitionTime.year);
  assertEqual(2001, storage.getTransition(2)->transitionTime.year);
  assertEqual(2002, storage.getTransition(3)->transitionTime.year);
}

test(TransitionStorageTest, addActiveCandidatesToActivePool) {
  TransitionStorage storage;
  storage.init();

  // create Prior to make it interesting
  Transition** prior = storage.reservePrior();
  (*prior)->transitionTime = {1999, 0, 1, 2, ZoneContext::kSuffixW};
  (*prior)->matchStatus = MatchStatus::kWithinMatch;

  // Add 3 transitions to Candidate pool, 2 active, 1 inactive.
  Transition* freeAgent = storage.getFreeAgent();
  freeAgent->transitionTime = {2000, 1, 2, 3, ZoneContext::kSuffixW};
  freeAgent->matchStatus = MatchStatus::kWithinMatch;
  storage.addFreeAgentToCandidatePool();

  freeAgent = storage.getFreeAgent();
  freeAgent->transitionTime = {2002, 3, 4, 5, ZoneContext::kSuffixW};
  freeAgent->matchStatus = MatchStatus::kWithinMatch;
  storage.addFreeAgentToCandidatePool();

  freeAgent = storage.getFreeAgent();
  freeAgent->transitionTime = {2001, 2, 3, 4, ZoneContext::kSuffixW};
  freeAgent->matchStatus = MatchStatus::kFarPast;
  storage.addFreeAgentToCandidatePool();

  // Add prior into the Candidate pool.
  storage.addPriorToCandidatePool();

  // Add the actives to the Active pool.
  storage.addActiveCandidatesToActivePool();

  // Verify that there are 3 transitions in the Active pool.
  assertEqual(3, storage.mIndexPrior);
  assertEqual(3, storage.mIndexCandidates);
  assertEqual(3, storage.mIndexFree);
  assertEqual(1999, storage.getTransition(0)->transitionTime.year);
  assertEqual(2000, storage.getTransition(1)->transitionTime.year);
  assertEqual(2002, storage.getTransition(2)->transitionTime.year);
}

test(TransitionStorageTest, resetCandidatePool) {
  TransitionStorage storage;
  storage.init();

  // Add 2 transitions to Candidate pool, 2 active, 1 inactive.
  Transition* freeAgent = storage.getFreeAgent();
  freeAgent->transitionTime = {2000, 1, 2, 3, ZoneContext::kSuffixW};
  freeAgent->matchStatus = MatchStatus::kWithinMatch;
  storage.addFreeAgentToCandidatePool();
  assertEqual(0, storage.mIndexPrior);
  assertEqual(0, storage.mIndexCandidates);
  assertEqual(1, storage.mIndexFree);

  freeAgent = storage.getFreeAgent();
  freeAgent->transitionTime = {2002, 3, 4, 5, ZoneContext::kSuffixW};
  freeAgent->matchStatus = MatchStatus::kWithinMatch;
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

  // Non-active can be added to the candidate pool.
  freeAgent = storage.getFreeAgent();
  freeAgent->transitionTime = {2001, 2, 3, 4, ZoneContext::kSuffixW};
  freeAgent->matchStatus = MatchStatus::kFarPast;
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

//---------------------------------------------------------------------------

test(TransitionStorageTest, findTransitionForSeconds) {
  TransitionStorage storage;
  using TransitionForSeconds = TransitionStorage::TransitionForSeconds;
  storage.init();

  // Add 3 transitions to Active pool.
  Transition* freeAgent = storage.getFreeAgent();
  freeAgent->transitionTime = {2000, 1, 2, 3, ZoneContext::kSuffixW};
  freeAgent->matchStatus = MatchStatus::kWithinMatch;
  freeAgent->startEpochSeconds = 2000000; // synthetic epochSeconds
  storage.addFreeAgentToCandidatePool();

  freeAgent = storage.getFreeAgent();
  freeAgent->transitionTime = {2001, 2, 3, 4, ZoneContext::kSuffixW};
  freeAgent->matchStatus = MatchStatus::kWithinMatch;
  freeAgent->startEpochSeconds = 2001000; // synthetic epochSeconds
  storage.addFreeAgentToCandidatePool();

  freeAgent = storage.getFreeAgent();
  freeAgent->transitionTime = {2002, 3, 4, 5, ZoneContext::kSuffixW};
  freeAgent->matchStatus = MatchStatus::kWithinMatch;
  freeAgent->startEpochSeconds = 2002000; // synthetic epochSeconds
  storage.addFreeAgentToCandidatePool();

  // Add the actives to the Active pool.
  storage.addActiveCandidatesToActivePool();

  // Check that we can find the transitions using the startEpochSeconds.

  // epochSeconds=1 far past
  TransitionForSeconds transitionForSeconds =
      storage.findTransitionForSeconds(1);
  const Transition* t = transitionForSeconds.curr;
  assertEqual(t, nullptr);

  // epochSeconds=2000001 found
  transitionForSeconds = storage.findTransitionForSeconds(2000001);
  t = transitionForSeconds.curr;
  assertEqual(2000, t->transitionTime.year);

  // epochSeconds=2001000 found
  transitionForSeconds = storage.findTransitionForSeconds(2001000);
  t = transitionForSeconds.curr;
  assertEqual(2001, t->transitionTime.year);

  // epochSeconds=2002000 found
  transitionForSeconds = storage.findTransitionForSeconds(2002000);
  t = transitionForSeconds.curr;
  assertEqual(2002, t->transitionTime.year);

  // epochSeconds=3000000 far future, matches the last transition
  transitionForSeconds = storage.findTransitionForSeconds(3000000);
  t = transitionForSeconds.curr;
  assertNotEqual(t, nullptr);
  assertEqual(2002, t->transitionTime.year);
}

test(TransitionStorageTest, findTransitionForDateTime) {
  TransitionStorage storage;
  storage.init();

  // Transition 1: [2000-01-02 01:00, 2001-04-01 02:00), before spring forward
  Transition* freeAgent = storage.getFreeAgent();
  freeAgent->transitionTime = {2000, 1, 2, 1*60, ZoneContext::kSuffixW};
  freeAgent->startDateTime = freeAgent->transitionTime;
  freeAgent->untilDateTime = {2001, 4, 1, 2*60, ZoneContext::kSuffixW};
  freeAgent->matchStatus = MatchStatus::kWithinMatch;
  storage.addFreeAgentToCandidatePool();

  // Transition 2: [2001-04-01 03:00, 2002-10-27 02:00), spring forward to
  // fall back, creating a gap from Transition 1.
  freeAgent = storage.getFreeAgent();
  freeAgent->transitionTime = {2001, 4, 1, 3*60, ZoneContext::kSuffixW};
  freeAgent->startDateTime = freeAgent->transitionTime;
  freeAgent->untilDateTime = {2002, 10, 27, 2*60, ZoneContext::kSuffixW};
  freeAgent->matchStatus = MatchStatus::kWithinMatch;
  storage.addFreeAgentToCandidatePool();

  // Transition 3: [2002-10-27 01:00, 2003-12-31 00:00), after fall back,
  // creating an overlap with Transition 2.
  freeAgent = storage.getFreeAgent();
  freeAgent->transitionTime = {2002, 10, 27, 1*60, ZoneContext::kSuffixW};
  freeAgent->startDateTime = freeAgent->transitionTime;
  freeAgent->untilDateTime = {2003, 12, 13, 0, ZoneContext::kSuffixW};
  freeAgent->matchStatus = MatchStatus::kWithinMatch;
  storage.addFreeAgentToCandidatePool();

  // Add the actives to the Active pool.
  storage.addActiveCandidatesToActivePool();

  assertEqual(3, storage.mIndexPrior);
  assertEqual(3, storage.mIndexCandidates);
  assertEqual(3, storage.mIndexFree);

  // 2000-01-01 00:00, far past
  auto ldt = LocalDateTime::forComponents(2000, 1, 1, 0, 0, 0);
  TransitionForDateTime r = storage.findTransitionForDateTime(ldt);
  assertEqual(r.num, 0);
  assertEqual(r.prev, nullptr);
  assertNotEqual(r.curr, nullptr);

  // 2000-01-30 01:00, matches Transition 1
  ldt = LocalDateTime::forComponents(2000, 1, 30, 1, 0, 0);
  r = storage.findTransitionForDateTime(ldt);
  assertEqual(r.num, 1);
  assertNotEqual(r.prev, nullptr);
  assertNotEqual(r.curr, nullptr);
  assertEqual(r.prev, r.curr);
  assertEqual(2000, r.curr->transitionTime.year);

  // 2001-04-01 02:30 is in the gap between Transition 1 and 2
  ldt = LocalDateTime::forComponents(2001, 4, 1, 2, 30, 0);
  r = storage.findTransitionForDateTime(ldt);
  assertEqual(r.num, 0);
  assertNotEqual(r.prev, nullptr);
  assertNotEqual(r.curr, nullptr);
  assertNotEqual(r.prev, r.curr);
  assertEqual(2000, r.prev->transitionTime.year);
  assertEqual(2001, r.curr->transitionTime.year);

  // 2002-10-27 01:30 is in the overlap between Transition 2 and 3
  ldt = LocalDateTime::forComponents(2002, 10, 27, 1, 30, 0);
  r = storage.findTransitionForDateTime(ldt);
  assertEqual(r.num, 2);
  assertNotEqual(r.prev, nullptr);
  assertNotEqual(r.curr, nullptr);
  assertNotEqual(r.prev, r.curr);
  assertEqual(2001, r.prev->transitionTime.year);
  assertEqual(2002, r.curr->transitionTime.year);

  // 2003-01-01 01:00 matches only Transition 3
  ldt = LocalDateTime::forComponents(2003, 1, 1, 1, 0, 0);
  r = storage.findTransitionForDateTime(ldt);
  assertEqual(r.num, 1);
  assertNotEqual(r.prev, nullptr);
  assertNotEqual(r.curr, nullptr);
  assertEqual(r.prev, r.curr);
  assertEqual(2002, r.curr->transitionTime.year);

  // 2005-01-01 00:00, far future
  ldt = LocalDateTime::forComponents(2005, 1, 1, 0, 0, 0);
  r = storage.findTransitionForDateTime(ldt);
  assertEqual(r.num, 0);
  assertNotEqual(r.prev, nullptr);
  assertEqual(r.curr, nullptr);
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
