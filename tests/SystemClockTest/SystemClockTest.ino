#line 2 "SystemClockTest.ino"

#include <AUnitVerbose.h>
#include <AceRoutine.h> // enable SystemClockCoroutine
#include <AceTime.h>
#include <ace_time/testing/FakeClock.h>
#include <ace_time/testing/TestableSystemClockLoop.h>
#include <ace_time/testing/TestableSystemClockCoroutine.h>

#define SYSTEM_CLOCK_TEST_DEBUG 0

using namespace aunit;
using namespace ace_time;
using namespace ace_time::clock;
using namespace ace_time::testing;

//---------------------------------------------------------------------------

// Verify that LocalTime::kInvalidSeconds is returned upon error
test(SystemClockLoopTest, invalidSeconds) {
  TestableClockInterface::setMillis(0);
  FakeClock backupAndReferenceClock;
  TestableSystemClockLoop systemClock(
      &backupAndReferenceClock, &backupAndReferenceClock);
  acetime_t now = systemClock.getNow();
  assertEqual(LocalTime::kInvalidSeconds, now);
}

//---------------------------------------------------------------------------

class SystemClockLoopTest: public TestOnce {
  protected:
    void setup() override {
      TestableClockInterface::setMillis(0);
      backupAndReferenceClock.init();
      systemClock.initSystemClock(
          &backupAndReferenceClock, &backupAndReferenceClock);
      systemClock.setup();
    }

    FakeClock backupAndReferenceClock; // backup and sync time keeper
    TestableSystemClockLoop systemClock;
};

testF(SystemClockLoopTest, setup) {
  assertEqual((acetime_t) 0, systemClock.getNow());

  // Run setup() a second time and verify expected result
  backupAndReferenceClock.setNow(100);
  systemClock.setup();
  assertEqual((acetime_t) 100, systemClock.getNow());
  assertEqual(SystemClock::kSyncStatusUnknown, systemClock.getSyncStatusCode());
}

testF(SystemClockLoopTest, backupNow) {
  assertEqual((acetime_t) 0, systemClock.getNow());
  assertEqual((acetime_t) 0, backupAndReferenceClock.getNow());

  unsigned long nowMillis = 1;
  TestableClockInterface::setMillis(nowMillis);
  systemClock.setNow(100);

  // setNow() caused a save to the backupClock which happens to be the
  // same as the referenceClock
  assertEqual((acetime_t) 100, systemClock.getNow());
  assertEqual((acetime_t) 100, backupAndReferenceClock.getNow());
}

testF(SystemClockLoopTest, syncNow) {
  assertEqual((acetime_t) 0, systemClock.getNow());
  assertEqual((acetime_t) 0, systemClock.getLastSyncTime());
  assertEqual((acetime_t) 0, backupAndReferenceClock.getNow());
  assertEqual((int16_t) 0, systemClock.getClockSkew());

  unsigned long nowMillis = 1;
  TestableClockInterface::setMillis(nowMillis);
  systemClock.syncNow(100);

  // syncNow() does NOT write to backupClock because the sync and backup
  // sources are identical, so it does not try to write the time back into
  // itself.
  assertEqual((acetime_t) 100, systemClock.getNow());
  assertEqual((acetime_t) 100, systemClock.getLastSyncTime());
  assertEqual((acetime_t) 0, backupAndReferenceClock.getNow());
  // The clock was skewed by 100 seconds, because the test clock does not
  // auto-advance.
  assertEqual((int16_t) -100, systemClock.getClockSkew());
}

testF(SystemClockLoopTest, getNow) {
  unsigned long nowMillis = 1;

  TestableClockInterface::setMillis(nowMillis);
  systemClock.setNow(100);
  assertEqual((acetime_t) 100, systemClock.getNow());

  // +900ms, no change to getNow()
  nowMillis += 900;
  TestableClockInterface::setMillis(nowMillis);
  assertEqual((acetime_t) 100, systemClock.getNow());

  // +100ms, getNow() should increase by 1
  nowMillis += 100;
  TestableClockInterface::setMillis(nowMillis);
  assertEqual((acetime_t) 101, systemClock.getNow());

  // +30000ms, getNow() should increase by 30
  nowMillis += 30000;
  TestableClockInterface::setMillis(nowMillis);
  assertEqual((acetime_t) 131, systemClock.getNow());

  // +40000ms, causing rollover of internal uint16_t version of millis, but
  // getNow() should still increase by another 40
  nowMillis += 40000;
  TestableClockInterface::setMillis(nowMillis);
  assertEqual((acetime_t) 171, systemClock.getNow());
}

// Test the error detection and retry algorithm of SystemClockLoop::loop(), with
// the FakeClock always returning an error.
testF(SystemClockLoopTest, loop) {
  unsigned long millis = 0;
  backupAndReferenceClock.isResponseReady(false);
  assertEqual(SystemClockLoop::kStatusReady, systemClock.mRequestStatus);
  assertEqual(SystemClock::kSyncStatusUnknown, systemClock.getSyncStatusCode());

  // retry with exponential backoff, doubling the delay on each
  // iteration, until we reach mSyncPeriodSeconds
  uint16_t expectedDelaySeconds = 5;
  bool firstRequestMade = false;
  for (; expectedDelaySeconds < systemClock.mSyncPeriodSeconds;
       expectedDelaySeconds *= 2) {

  #if SYSTEM_CLOCK_TEST_DEBUG >= 1
    Serial.print("expectedDelaySeconds: ");
    Serial.println(expectedDelaySeconds);
  #endif

    // t = 0, make a request and waits for response
    systemClock.loop();
    assertEqual(SystemClockLoop::kStatusSent, systemClock.mRequestStatus);
    assertEqual((int32_t) 0, systemClock.getSecondsSinceSyncAttempt());
    assertEqual((int32_t) expectedDelaySeconds,
        systemClock.getSecondsToSyncAttempt());
    assertEqual(
        firstRequestMade
            ? SystemClock::kSyncStatusTimedOut
            : SystemClock::kSyncStatusUnknown,
        systemClock.getSyncStatusCode());
    firstRequestMade = true;

    // t = +1 s, back off for mCurrentSyncPeriodSeconds
    for (uint16_t i = 1; i < expectedDelaySeconds; i++) {
      millis += 1000;
      TestableClockInterface::setMillis(millis);
      systemClock.loop();
      assertEqual(SystemClockLoop::kStatusWaitForRetry,
          systemClock.mRequestStatus);
      assertEqual((int32_t) i, systemClock.getSecondsSinceSyncAttempt());
      assertEqual((int32_t) expectedDelaySeconds - i,
          systemClock.getSecondsToSyncAttempt());
      assertEqual(
          SystemClock::kSyncStatusTimedOut,
          systemClock.getSyncStatusCode());
    }

    // t = +1 s, waiting over, go to kStatusReady to make another request
    millis += 1000;
    TestableClockInterface::setMillis(millis);
    systemClock.loop();
    assertEqual(SystemClockLoop::kStatusReady, systemClock.mRequestStatus);
    assertEqual((int32_t) expectedDelaySeconds,
        systemClock.getSecondsSinceSyncAttempt());
    assertEqual(0, systemClock.getSecondsToSyncAttempt());
    assertEqual(
        SystemClock::kSyncStatusTimedOut,
        systemClock.getSyncStatusCode());
  }

  // Last iteration. Make a request.
  systemClock.loop();
  assertEqual(SystemClockLoop::kStatusSent, systemClock.mRequestStatus);
  assertEqual(0, systemClock.getSecondsSinceSyncAttempt());
  assertEqual(systemClock.mCurrentSyncPeriodSeconds,
      systemClock.getSecondsToSyncAttempt());
  assertEqual(
      SystemClock::kSyncStatusTimedOut,
      systemClock.getSyncStatusCode());

  // wait for response
  millis += 1000;
  TestableClockInterface::setMillis(millis);
  systemClock.loop();
  assertEqual(SystemClockLoop::kStatusWaitForRetry, systemClock.mRequestStatus);
  assertEqual(
      SystemClock::kSyncStatusTimedOut,
      systemClock.getSyncStatusCode());

  // Final wait for 3600 seconds
  expectedDelaySeconds = systemClock.mSyncPeriodSeconds;
  assertEqual(expectedDelaySeconds, systemClock.mCurrentSyncPeriodSeconds);
  for (uint16_t i = 1; i < expectedDelaySeconds - 1; i++) {
    millis += 1000;
    TestableClockInterface::setMillis(millis);
    systemClock.loop();
    assertEqual(SystemClockLoop::kStatusWaitForRetry,
        systemClock.mRequestStatus);
    assertEqual(
        SystemClock::kSyncStatusTimedOut,
        systemClock.getSyncStatusCode());
  }

  // Let the loop timeout and go into a ready state.
  millis += 1000;
  TestableClockInterface::setMillis(millis);
  systemClock.loop();
  assertEqual(SystemClockLoop::kStatusReady, systemClock.mRequestStatus);
  assertEqual(
      SystemClock::kSyncStatusTimedOut,
      systemClock.getSyncStatusCode());

  // Make a new request
  millis += 1000;
  TestableClockInterface::setMillis(millis);
  systemClock.loop();
  assertEqual(SystemClockLoop::kStatusSent, systemClock.mRequestStatus);
  assertEqual(0, systemClock.getSecondsSinceSyncAttempt());
  assertEqual(systemClock.mCurrentSyncPeriodSeconds,
      systemClock.getSecondsToSyncAttempt());
  assertEqual(
      SystemClock::kSyncStatusTimedOut,
      systemClock.getSyncStatusCode());

  // Check 1ms later for a successful request
  backupAndReferenceClock.isResponseReady(true);
  backupAndReferenceClock.setNow(42);
  millis += 1;
  TestableClockInterface::setMillis(millis);
  systemClock.loop();
  assertEqual(SystemClockLoop::kStatusOk, systemClock.mRequestStatus);
  assertEqual((acetime_t) 42, systemClock.getNow());
  assertEqual((acetime_t) 42, systemClock.getLastSyncTime());
  assertTrue(systemClock.isInit());
  assertEqual(0, systemClock.getSecondsSinceSyncAttempt());
  assertEqual(systemClock.mCurrentSyncPeriodSeconds - 1 /* 1 ms later */,
      systemClock.getSecondsToSyncAttempt());
  assertEqual(SystemClock::kSyncStatusOk, systemClock.getSyncStatusCode());
}

//---------------------------------------------------------------------------

// Currently only one test uses this class, so strictly this isn't necessary
// but it's good to use the same pattern as SystemClockLoopTest.
class SystemClockCoroutineTest: public TestOnce {
  protected:
    void setup() override {
      TestableClockInterface::setMillis(0);
      backupAndReferenceClock.init();
      systemClock.initSystemClock(
          &backupAndReferenceClock, &backupAndReferenceClock);
      systemClock.setup();
    }

    FakeClock backupAndReferenceClock;
    TestableSystemClockCoroutine systemClock;
};

testF(SystemClockCoroutineTest, runCoroutine) {
  unsigned long millis = 0;
  backupAndReferenceClock.isResponseReady(false);
  assertEqual(SystemClockCoroutine::kStatusUnknown, systemClock.mRequestStatus);
  assertEqual(
      SystemClock::kSyncStatusUnknown,
      systemClock.getSyncStatusCode());

  // retry with exponential backoff 10 times, doubling the delay on each
  // iteration
  uint16_t expectedDelaySeconds = 5;
  bool firstRequestMade = false;
  for (; expectedDelaySeconds < systemClock.mSyncPeriodSeconds;
       expectedDelaySeconds *= 2) {

#if SYSTEM_CLOCK_TEST_DEBUG >= 1
    Serial.print("expectedDelaySeconds: ");
    Serial.println(expectedDelaySeconds);
#endif

    // t = 0, sends request and waits for response
    systemClock.runCoroutine();
    assertTrue(systemClock.isYielding());
    assertEqual(0, systemClock.getSecondsSinceSyncAttempt());
    assertEqual(systemClock.mCurrentSyncPeriodSeconds,
        systemClock.getSecondsToSyncAttempt());
    assertEqual(
        firstRequestMade
            ? SystemClock::kSyncStatusTimedOut
            : SystemClock::kSyncStatusUnknown,
        systemClock.getSyncStatusCode());
    firstRequestMade = true;

    // t = +1 s, request timed out, delay for 'expectedDelaySeconds'
    for (uint16_t i = 0; i < expectedDelaySeconds; i++) {
      millis += 1000;
      TestableClockInterface::setMillis(millis);
      systemClock.runCoroutine();
      assertTrue(systemClock.isDelaying());
      assertEqual((int32_t) i + 1, systemClock.getSecondsSinceSyncAttempt());
      assertEqual((int32_t) expectedDelaySeconds - i,
          systemClock.getSecondsToSyncAttempt());
      assertEqual(
          SystemClock::kSyncStatusTimedOut,
          systemClock.getSyncStatusCode());
    }

    // t = +1 s, prepare to make another request
    millis += 1000;
    TestableClockInterface::setMillis(millis);
  }

  // Make one final request
  systemClock.runCoroutine();
  assertTrue(systemClock.isYielding());
  assertEqual(SystemClockCoroutine::kStatusSent, systemClock.mRequestStatus);
  assertEqual(0, systemClock.getSecondsSinceSyncAttempt());
  assertEqual(systemClock.mCurrentSyncPeriodSeconds,
      systemClock.getSecondsToSyncAttempt());
  assertEqual(
      SystemClock::kSyncStatusTimedOut,
      systemClock.getSyncStatusCode());

  // Repeatedly check for final delay of 3600
  expectedDelaySeconds = systemClock.mSyncPeriodSeconds;
  for (uint16_t i = 1; i <= expectedDelaySeconds; i++) {
    millis += 1000;
    TestableClockInterface::setMillis(millis);
    systemClock.runCoroutine();
    assertTrue(systemClock.isDelaying());
    assertEqual(
        SystemClock::kSyncStatusTimedOut,
        systemClock.getSyncStatusCode());
  }

  // Set up to make a successful request
  millis += 1000;
  TestableClockInterface::setMillis(millis);
  backupAndReferenceClock.isResponseReady(true);
  backupAndReferenceClock.setNow(42);

  // verify successful request
  systemClock.runCoroutine();
  assertTrue(systemClock.isDelaying());
  assertEqual(systemClock.mRequestStatus, SystemClockCoroutine::kStatusOk);
  assertEqual((acetime_t) 42, systemClock.getNow());
  assertEqual((acetime_t) 42, systemClock.getLastSyncTime());
  assertTrue(systemClock.isInit());
  assertEqual(0, systemClock.getSecondsSinceSyncAttempt());
  assertEqual(systemClock.mCurrentSyncPeriodSeconds,
      systemClock.getSecondsToSyncAttempt());
  assertEqual(SystemClock::kSyncStatusOk, systemClock.getSyncStatusCode());
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
