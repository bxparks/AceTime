#line 2 "TransitionStorageTest.ino"

/*
 * Unit tests for TransitionStorage, extracted from ExtendedZoneProcessorTest
 * after it became too big to run on an Arduino/SparkFun ProMicro environment.
 */

#include <AUnit.h>
#include <AceTime.h>

using namespace aunit;
using namespace ace_time;
using namespace ace_time::extended;

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

test(TransitionStorageTest, setFreeAgentAsPrior) {
  TransitionStorage<4> storage;
  storage.init();

  Transition** priorReservation = storage.reservePrior();
  (*priorReservation)->active = false;
  Transition* freeAgent = storage.getFreeAgent();
  freeAgent->active = true;
  storage.setFreeAgentAsPrior();

  // Verify that the two have been swapped.
  Transition* prior = storage.getPrior();
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
  freeAgent->transitionTime = {0, 1, 2, 3, ZoneContext::kSuffixW};
  storage.addFreeAgentToCandidatePool();
  assertEqual(0, storage.mIndexPrior);
  assertEqual(1, storage.mIndexCandidates);
  assertEqual(2, storage.mIndexFree);

  freeAgent = storage.getFreeAgent();
  freeAgent->transitionTime = {2, 3, 4, 5, ZoneContext::kSuffixW};
  storage.addFreeAgentToCandidatePool();
  assertEqual(0, storage.mIndexPrior);
  assertEqual(1, storage.mIndexCandidates);
  assertEqual(3, storage.mIndexFree);

  freeAgent = storage.getFreeAgent();
  freeAgent->transitionTime = {1, 2, 3, 4, ZoneContext::kSuffixW};
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
  Transition** prior = storage.reservePrior();
  (*prior)->transitionTime = {-1, 0, 1, 2, ZoneContext::kSuffixW};
  (*prior)->active = true;

  // Add 3 transitions to Candidate pool, 2 active, 1 inactive.
  Transition* freeAgent = storage.getFreeAgent();
  freeAgent->transitionTime = {0, 1, 2, 3, ZoneContext::kSuffixW};
  freeAgent->active = true;
  storage.addFreeAgentToCandidatePool();

  freeAgent = storage.getFreeAgent();
  freeAgent->transitionTime = {2, 3, 4, 5, ZoneContext::kSuffixW};
  freeAgent->active = true;
  storage.addFreeAgentToCandidatePool();

  freeAgent = storage.getFreeAgent();
  freeAgent->transitionTime = {1, 2, 3, 4, ZoneContext::kSuffixW};
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

  // Add 3 transitions to Active pool.
  Transition* freeAgent = storage.getFreeAgent();
  freeAgent->transitionTime = {0, 1, 2, 3, ZoneContext::kSuffixW};
  freeAgent->active = true;
  freeAgent->startEpochSeconds = 0;
  storage.addFreeAgentToCandidatePool();

  freeAgent = storage.getFreeAgent();
  freeAgent->transitionTime = {1, 2, 3, 4, ZoneContext::kSuffixW};
  freeAgent->active = true;
  freeAgent->startEpochSeconds = 10;
  storage.addFreeAgentToCandidatePool();

  freeAgent = storage.getFreeAgent();
  freeAgent->transitionTime = {2, 3, 4, 5, ZoneContext::kSuffixW};
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

test(TransitionStorageTest, findTransitionForDateTime) {
  TransitionStorage<4> storage;
  storage.init();

  // 2000-01-02T00:03
  Transition* freeAgent = storage.getFreeAgent();
  freeAgent->transitionTime = {0, 1, 2, 3, ZoneContext::kSuffixW};
  freeAgent->startDateTime = freeAgent->transitionTime;
  freeAgent->active = true;
  storage.addFreeAgentToCandidatePool();

  // 2001-02-03T00:04
  freeAgent = storage.getFreeAgent();
  freeAgent->transitionTime = {1, 2, 3, 4, ZoneContext::kSuffixW};
  freeAgent->startDateTime = freeAgent->transitionTime;
  freeAgent->active = true;
  storage.addFreeAgentToCandidatePool();

  // 2002-03-04T00:05
  freeAgent = storage.getFreeAgent();
  freeAgent->transitionTime = {2, 3, 4, 5, ZoneContext::kSuffixW};
  freeAgent->startDateTime = freeAgent->transitionTime;
  freeAgent->active = true;
  storage.addFreeAgentToCandidatePool();

  // Add the actives to the Active pool.
  storage.addActiveCandidatesToActivePool();

  assertEqual(3, storage.mIndexPrior);
  assertEqual(3, storage.mIndexCandidates);
  assertEqual(3, storage.mIndexFree);

  // 2000-01-02T00:00
  auto ldt = LocalDateTime::forComponents(2000, 1, 2, 0, 0, 0);
  const Transition* t = storage.findTransitionForDateTime(ldt);
  assertEqual(t, nullptr);

  // 2000-01-02T01:00
  ldt = LocalDateTime::forComponents(2000, 1, 2, 1, 0, 0);
  t = storage.findTransitionForDateTime(ldt);
  assertNotEqual(t, nullptr);
  assertEqual(0, t->transitionTime.yearTiny);

  // 2001-02-03T00:03
  ldt = LocalDateTime::forComponents(2001, 2, 3, 0, 3, 0);
  t = storage.findTransitionForDateTime(ldt);
  assertNotEqual(t, nullptr);
  assertEqual(0, t->transitionTime.yearTiny);

  // 2001-02-03T00:04
  ldt = LocalDateTime::forComponents(2001, 2, 3, 0, 4, 0);
  t = storage.findTransitionForDateTime(ldt);
  assertNotEqual(t, nullptr);
  assertEqual(1, t->transitionTime.yearTiny);

  // 2002-03-04T00:05
  ldt = LocalDateTime::forComponents(2002, 3, 4, 0, 5, 0);
  t = storage.findTransitionForDateTime(ldt);
  assertNotEqual(t, nullptr);
  assertEqual(2, t->transitionTime.yearTiny);
}

test(TransitionStorageTest, resetCandidatePool) {
  TransitionStorage<4> storage;
  storage.init();

  // Add 2 transitions to Candidate pool, 2 active, 1 inactive.
  Transition* freeAgent = storage.getFreeAgent();
  freeAgent->transitionTime = {0, 1, 2, 3, ZoneContext::kSuffixW};
  freeAgent->active = true;
  storage.addFreeAgentToCandidatePool();
  assertEqual(0, storage.mIndexPrior);
  assertEqual(0, storage.mIndexCandidates);
  assertEqual(1, storage.mIndexFree);

  freeAgent = storage.getFreeAgent();
  freeAgent->transitionTime = {2, 3, 4, 5, ZoneContext::kSuffixW};
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
  freeAgent->transitionTime = {1, 2, 3, 4, ZoneContext::kSuffixW};
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
