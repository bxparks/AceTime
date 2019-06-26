#line 2 "SystemClockTest.ino"

#include <AUnitVerbose.h>
#include <AceRoutine.h> // enable SystemClockSyncCoroutine
#include <AceTime.h>
#include <ace_time/testing/FakeMillis.h>
#include <ace_time/testing/FakeTimeKeeper.h>
#include <ace_time/testing/TestableSystemClock.h>
#include <ace_time/testing/TestableSystemClockSyncCoroutine.h>

using namespace aunit;
using namespace ace_time;
using namespace ace_time::clock;
using namespace ace_time::common;
using namespace ace_time::testing;

//---------------------------------------------------------------------------

// Verify that LocalTime::kInvalidSeconds is returned upon error
test(SystemClockTest, invalidSeconds) {
  FakeMillis* fakeMillis = new FakeMillis();
  FakeTimeKeeper* backupAndSyncTimeKeeper = new FakeTimeKeeper();
  TestableSystemClock* systemClock = new TestableSystemClock(
      backupAndSyncTimeKeeper, backupAndSyncTimeKeeper, fakeMillis);
  acetime_t now = systemClock->getNow();
  assertEqual(LocalTime::kInvalidSeconds, now);
}

//---------------------------------------------------------------------------

class SystemClockTest: public TestOnce {
  protected:
    void setup() override {
      fakeMillis = new FakeMillis();
      backupAndSyncTimeKeeper = new FakeTimeKeeper();
      systemClock = new TestableSystemClock(
          backupAndSyncTimeKeeper, backupAndSyncTimeKeeper, fakeMillis);

      systemClock->setup();
    }

    void teardown() override {
      delete systemClock;
      delete backupAndSyncTimeKeeper;
      delete fakeMillis;
    }

    FakeMillis* fakeMillis;
    FakeTimeKeeper* backupAndSyncTimeKeeper; // backup and sync time keeper
    TestableSystemClock* systemClock;
};

testF(SystemClockTest, setup) {
  assertEqual((acetime_t) 0, systemClock->getNow());

  backupAndSyncTimeKeeper->setNow(100);
  systemClock->setup();
  assertEqual((acetime_t) 100, systemClock->getNow());
}

testF(SystemClockTest, backupNow) {
  assertEqual((acetime_t) 0, systemClock->getNow());
  assertEqual((acetime_t) 0, backupAndSyncTimeKeeper->getNow());

  unsigned long nowMillis = 1;
  fakeMillis->millis(nowMillis);
  systemClock->setNow(100);

  // setNow() caused a save to the backupTimeKeeper which happens to be the
  // same backupAndSyncTimeKeeper
  assertEqual((acetime_t) 100, systemClock->getNow());
  assertEqual((acetime_t) 100, backupAndSyncTimeKeeper->getNow());
}

testF(SystemClockTest, sync) {
  assertEqual((acetime_t) 0, systemClock->getNow());
  assertEqual((acetime_t) 0, systemClock->getLastSyncTime());
  assertEqual((acetime_t) 0, backupAndSyncTimeKeeper->getNow());

  unsigned long nowMillis = 1;
  fakeMillis->millis(nowMillis);
  systemClock->sync(100);

  // sync() does NOT write to backupTimeKeeper because the sync and backup
  // sources are identical, so it does not try to write the time back into
  // itself.
  assertEqual((acetime_t) 100, systemClock->getNow());
  assertEqual((acetime_t) 100, systemClock->getLastSyncTime());
  assertEqual((acetime_t) 0, backupAndSyncTimeKeeper->getNow());
}

testF(SystemClockTest, getNow) {
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

//---------------------------------------------------------------------------

// Create dedicated test class to allow friend access to private members of
// SystemClockSyncCoroutine for testing purposes.
class SystemClockSyncCoroutineTest: public TestOnce {
  protected:
    void setup() override {
      fakeMillis = new FakeMillis();
      backupAndSyncTimeKeeper = new FakeTimeKeeper();
      systemClock = new TestableSystemClock(
          backupAndSyncTimeKeeper, backupAndSyncTimeKeeper, fakeMillis);
      systemClockSyncCoroutine = new TestableSystemClockSyncCoroutine(
          *systemClock, fakeMillis);

      systemClock->setup();
      systemClockSyncCoroutine->setupCoroutine("systemClockSyncCoroutine");
    }

    void teardown() override {
      delete systemClockSyncCoroutine;
      delete systemClock;
      delete backupAndSyncTimeKeeper;
      delete fakeMillis;
    }

    void assertRunCoroutine() {
      unsigned long millis = 0;
      backupAndSyncTimeKeeper->isResponseReady(false);

      // t = 0, sends request and waits for response
      systemClockSyncCoroutine->runCoroutine();
      assertTrue(systemClockSyncCoroutine->isYielding());

      // retry with exponential backoff 10 times, doubling the delay on each
      // iteration
      uint16_t expectedDelaySeconds = 5;
      for (int retries = 0; retries < 10; retries++) {
        // t = +1 s, request timed out, delay for 5 sec
        for (uint16_t i = 1; i <= expectedDelaySeconds; i++) {
          millis += 1000;
          fakeMillis->millis(millis);
          systemClockSyncCoroutine->runCoroutine();
          assertTrue(systemClockSyncCoroutine->isDelaying());
        }

        // t = +1 s, make another request and wait for response
        millis += 1000;
        fakeMillis->millis(millis);
        systemClockSyncCoroutine->runCoroutine();
        assertTrue(systemClockSyncCoroutine->isYielding());

        expectedDelaySeconds *= 2;
      }

      // Final delay of 3600
      expectedDelaySeconds = 3600;

      // t = +1 s, request timed out, delay for 5 sec
      for (uint16_t i = 1; i <= expectedDelaySeconds; i++) {
        millis += 1000;
        fakeMillis->millis(millis);
        systemClockSyncCoroutine->runCoroutine();
        assertTrue(systemClockSyncCoroutine->isDelaying());
      }

      // Make a new request and let it succeed
      millis += 1000;
      fakeMillis->millis(millis);
      backupAndSyncTimeKeeper->isResponseReady(true);
      backupAndSyncTimeKeeper->setNow(42);

      // verify successful request
      systemClockSyncCoroutine->runCoroutine();
      assertTrue(systemClockSyncCoroutine->isDelaying());
      assertEqual(systemClockSyncCoroutine->mRequestStatus,
          SystemClockSyncCoroutine::kStatusOk);
      assertEqual((acetime_t) 42, systemClock->getNow());
      assertEqual((acetime_t) 42, systemClock->getLastSyncTime());
      assertTrue(systemClock->isInit());
    }

    FakeMillis* fakeMillis;
    FakeTimeKeeper* backupAndSyncTimeKeeper;
    TestableSystemClock* systemClock;
    SystemClockSyncCoroutine* systemClockSyncCoroutine;
};

testF(SystemClockSyncCoroutineTest, sync) {
  assertRunCoroutine();
}

//---------------------------------------------------------------------------

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
