#line 2 "SystemClockTest.ino"

#include <AUnitVerbose.h>
#include <AceRoutine.h> // enable SystemClockCoroutine
#include <AceTime.h>
#include <ace_time/testing/FakeMillis.h>
#include <ace_time/testing/FakeClock.h>
#include <ace_time/testing/TestableSystemClockLoop.h>
#include <ace_time/testing/TestableSystemClockCoroutine.h>

#define SYSTEM_CLOCK_TEST_DEBUG 0

using namespace aunit;
using namespace ace_time;
using namespace ace_time::clock;
using namespace ace_time::common;
using namespace ace_time::testing;

//---------------------------------------------------------------------------

// Verify that LocalTime::kInvalidSeconds is returned upon error
test(SystemClockLoopTest, invalidSeconds) {
  FakeMillis* fakeMillis = new FakeMillis();
  FakeClock* backupAndReferenceClock = new FakeClock();
  TestableSystemClockLoop* systemClock = new TestableSystemClockLoop(
      backupAndReferenceClock, backupAndReferenceClock, fakeMillis);
  acetime_t now = systemClock->getNow();
  assertEqual(LocalTime::kInvalidSeconds, now);
}

//---------------------------------------------------------------------------

class SystemClockLoopTest: public TestOnce {
  protected:
    void setup() override {
      fakeMillis = new FakeMillis();
      backupAndReferenceClock = new FakeClock();
      systemClock = new TestableSystemClockLoop(
          backupAndReferenceClock, backupAndReferenceClock, fakeMillis);
      systemClock->setup();
    }

    void teardown() override {
      delete systemClock;
      delete backupAndReferenceClock;
      delete fakeMillis;
    }

    FakeMillis* fakeMillis;
    FakeClock* backupAndReferenceClock; // backup and sync time keeper
    TestableSystemClockLoop* systemClock;
};

testF(SystemClockLoopTest, setup) {
  assertEqual((acetime_t) 0, systemClock->getNow());

  // Run setup() a second time and verify expected result
  backupAndReferenceClock->setNow(100);
  systemClock->setup();
  assertEqual((acetime_t) 100, systemClock->getNow());
}

testF(SystemClockLoopTest, backupNow) {
  assertEqual((acetime_t) 0, systemClock->getNow());
  assertEqual((acetime_t) 0, backupAndReferenceClock->getNow());

  unsigned long nowMillis = 1;
  fakeMillis->millis(nowMillis);
  systemClock->setNow(100);

  // setNow() caused a save to the backupClock which happens to be the
  // same as the referenceClock
  assertEqual((acetime_t) 100, systemClock->getNow());
  assertEqual((acetime_t) 100, backupAndReferenceClock->getNow());
}

testF(SystemClockLoopTest, syncNow) {
  assertEqual((acetime_t) 0, systemClock->getNow());
  assertEqual((acetime_t) 0, systemClock->getLastSyncTime());
  assertEqual((acetime_t) 0, backupAndReferenceClock->getNow());

  unsigned long nowMillis = 1;
  fakeMillis->millis(nowMillis);
  systemClock->syncNow(100);

  // syncNow() does NOT write to backupClock because the sync and backup
  // sources are identical, so it does not try to write the time back into
  // itself.
  assertEqual((acetime_t) 100, systemClock->getNow());
  assertEqual((acetime_t) 100, systemClock->getLastSyncTime());
  assertEqual((acetime_t) 0, backupAndReferenceClock->getNow());
}

testF(SystemClockLoopTest, getNow) {
  unsigned long nowMillis = 1;

  fakeMillis->millis(nowMillis);
  systemClock->setNow(100);
  assertEqual((acetime_t) 100, systemClock->getNow());

  // +900ms, no change to getNow()
  nowMillis += 900;
  fakeMillis->millis(nowMillis);
  assertEqual((acetime_t) 100, systemClock->getNow());

  // +100ms, getNow() should increase by 1
  nowMillis += 100;
  fakeMillis->millis(nowMillis);
  assertEqual((acetime_t) 101, systemClock->getNow());

  // +30000ms, getNow() should increase by 30
  nowMillis += 30000;
  fakeMillis->millis(nowMillis);
  assertEqual((acetime_t) 131, systemClock->getNow());

  // +40000ms, causing rollover of internal uint16_t version of millis, but
  // getNow() should still increase by another 40
  nowMillis += 40000;
  fakeMillis->millis(nowMillis);
  assertEqual((acetime_t) 171, systemClock->getNow());
}

testF(SystemClockLoopTest, loop) {
  unsigned long millis = 0;
  backupAndReferenceClock->isResponseReady(false);
  assertEqual(SystemClockLoop::kStatusReady, systemClock->mRequestStatus);

  // retry with exponential backoff, doubling the delay on each
  // iteration, until we reach mSyncPeriodSeconds
  uint16_t expectedDelaySeconds = 5;
  for (; expectedDelaySeconds < systemClock->mSyncPeriodSeconds;
       expectedDelaySeconds *= 2) {

    // t = 0, make a request and waits for response
    systemClock->loop();
    assertEqual(SystemClockLoop::kStatusSent, systemClock->mRequestStatus);

    // t= 1, request timed out
    millis += 1000;
    fakeMillis->millis(millis);
    systemClock->loop();
    assertEqual(SystemClockLoop::kStatusWaitForRetry,
        systemClock->mRequestStatus);
    assertEqual(expectedDelaySeconds, systemClock->mCurrentSyncPeriodSeconds);

    // t = +1 s, back off for mCurrentSyncPeriodSeconds
    for (uint16_t i = 1; i < expectedDelaySeconds - 1; i++) {
      millis += 1000;
      fakeMillis->millis(millis);
      systemClock->loop();
      assertEqual(SystemClockLoop::kStatusWaitForRetry,
          systemClock->mRequestStatus);
    }

    // t = +1 s, waiting over, go to kStatusReady to make another request
    millis += 1000;
    fakeMillis->millis(millis);
    systemClock->loop();
    assertEqual(SystemClockLoop::kStatusReady, systemClock->mRequestStatus);
  }

  // Last iteration. Make a request.
  systemClock->loop();
  assertEqual(SystemClockLoop::kStatusSent, systemClock->mRequestStatus);
  // wait for response
  millis += 1000;
  fakeMillis->millis(millis);
  systemClock->loop();
  assertEqual(SystemClockLoop::kStatusWaitForRetry,
      systemClock->mRequestStatus);
  // Final wait for 3600 seconds
  expectedDelaySeconds = systemClock->mSyncPeriodSeconds;
  assertEqual(expectedDelaySeconds, systemClock->mCurrentSyncPeriodSeconds);
  for (uint16_t i = 1; i < expectedDelaySeconds - 1; i++) {
    millis += 1000;
    fakeMillis->millis(millis);
    systemClock->loop();
    assertEqual(SystemClockLoop::kStatusWaitForRetry,
        systemClock->mRequestStatus);
  }

  // Let the loop timeout and go into a ready state.
  millis += 1000;
  fakeMillis->millis(millis);
  systemClock->loop();
  assertEqual(SystemClockLoop::kStatusReady, systemClock->mRequestStatus);

  // Make a new request
  millis += 1000;
  fakeMillis->millis(millis);
  systemClock->loop();
  assertEqual(SystemClockLoop::kStatusSent, systemClock->mRequestStatus);

  // Check 1ms later for a successful request
  backupAndReferenceClock->isResponseReady(true);
  backupAndReferenceClock->setNow(42);
  millis += 1;
  fakeMillis->millis(millis);
  systemClock->loop();
  assertEqual(SystemClockLoop::kStatusOk, systemClock->mRequestStatus);
  assertEqual((acetime_t) 42, systemClock->getNow());
  assertEqual((acetime_t) 42, systemClock->getLastSyncTime());
  assertTrue(systemClock->isInit());
}

//---------------------------------------------------------------------------

// Currently only one test uses this class, so strictly this isn't necessary
// but it's good to use the same pattern as SystemClockLoopTest.
class SystemClockCoroutineTest: public TestOnce {
  protected:
    void setup() override {
      fakeMillis = new FakeMillis();
      backupAndReferenceClock = new FakeClock();
      systemClock = new TestableSystemClockCoroutine(
          backupAndReferenceClock, backupAndReferenceClock, fakeMillis);
      systemClock->setup();
      systemClock->setupCoroutine("systemClockCoroutine");
    }

    void teardown() override {
      delete systemClock;
      delete backupAndReferenceClock;
      delete fakeMillis;
    }

    FakeMillis* fakeMillis;
    FakeClock* backupAndReferenceClock;
    TestableSystemClockCoroutine* systemClock;
};

testF(SystemClockCoroutineTest, runCoroutine) {
  unsigned long millis = 0;
  backupAndReferenceClock->isResponseReady(false);

  // retry with exponential backoff 10 times, doubling the delay on each
  // iteration
  uint16_t expectedDelaySeconds = 5;
#if SYSTEM_CLOCK_TEST_DEBUG
  int iter = 0;
#endif
  for (; expectedDelaySeconds < systemClock->mSyncPeriodSeconds;
       expectedDelaySeconds *= 2) {

#if SYSTEM_CLOCK_TEST_DEBUG
    logging::printf("Iteration %i\n", iter);
    iter++;
#endif

    // t = 0, sends request and waits for response
    systemClock->runCoroutine();
    assertTrue(systemClock->isYielding());

    // t = +1 s, request timed out, delay for 5 sec
    for (uint16_t i = 1; i <= expectedDelaySeconds; i++) {
      millis += 1000;
      fakeMillis->millis(millis);
      systemClock->runCoroutine();
      assertTrue(systemClock->isDelaying());
    }

    // t = +1 s, prepare to make another request
    millis += 1000;
    fakeMillis->millis(millis);
  }

  // Make one final request
  systemClock->runCoroutine();
  assertTrue(systemClock->isYielding());

  // Final delay of 3600
  expectedDelaySeconds = systemClock->mSyncPeriodSeconds;
  for (uint16_t i = 1; i <= expectedDelaySeconds; i++) {
    millis += 1000;
    fakeMillis->millis(millis);
    systemClock->runCoroutine();
    assertTrue(systemClock->isDelaying());
  }

  // Set up to make a successful request
  millis += 1000;
  fakeMillis->millis(millis);
  backupAndReferenceClock->isResponseReady(true);
  backupAndReferenceClock->setNow(42);

  // verify successful request
  systemClock->runCoroutine();
  assertTrue(systemClock->isDelaying());
  assertEqual(systemClock->mRequestStatus, SystemClockCoroutine::kStatusOk);
  assertEqual((acetime_t) 42, systemClock->getNow());
  assertEqual((acetime_t) 42, systemClock->getLastSyncTime());
  assertTrue(systemClock->isInit());
}

//---------------------------------------------------------------------------

void setup() {
#if ! defined(UNIX_HOST_DUINO)
  delay(1000); // wait to prevent garbage on SERIAL_PORT_MONITOR
#endif
  SERIAL_PORT_MONITOR.begin(115200);
  while(!SERIAL_PORT_MONITOR); // for the Arduino Leonardo/Micro only
}

void loop() {
  TestRunner::run();
}
