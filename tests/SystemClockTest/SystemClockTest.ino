#line 2 "SystemClockTest.ino"

#include <AUnitVerbose.h>
#include <AceRoutine.h> // enable SystemClockCoroutine
#include <AceTime.h>
#include <ace_time/testing/FakeMillis.h>
#include <ace_time/testing/FakeTimeKeeper.h>
#include <ace_time/testing/TestableSystemClockLoop.h>
#include <ace_time/testing/TestableSystemClockCoroutine.h>

using namespace aunit;
using namespace ace_time;
using namespace ace_time::clock;
using namespace ace_time::common;
using namespace ace_time::testing;

//---------------------------------------------------------------------------

// Verify that LocalTime::kInvalidSeconds is returned upon error
test(SystemClockLoopTest, invalidSeconds) {
  FakeMillis* fakeMillis = new FakeMillis();
  FakeTimeKeeper* backupAndReferenceClock = new FakeTimeKeeper();
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
      backupAndReferenceClock = new FakeTimeKeeper();
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
    FakeTimeKeeper* backupAndReferenceClock; // backup and sync time keeper
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

  // setNow() caused a save to the backupTimeKeeper which happens to be the
  // same backupAndReferenceClock
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

  // syncNow() does NOT write to backupTimeKeeper because the sync and backup
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

//---------------------------------------------------------------------------

// Currently only one test uses this class, so strictly this isn't necessary
// but it's good to use the same pattern as SystemClockLoopTest.
class SystemClockCoroutineTest: public TestOnce {
  protected:
    void setup() override {
      fakeMillis = new FakeMillis();
      backupAndReferenceClock = new FakeTimeKeeper();
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

    void assertRunCoroutine() {
      unsigned long millis = 0;
      backupAndReferenceClock->isResponseReady(false);

      // t = 0, sends request and waits for response
      systemClock->runCoroutine();
      assertTrue(systemClock->isYielding());

      // retry with exponential backoff 10 times, doubling the delay on each
      // iteration
      uint16_t expectedDelaySeconds = 5;
      for (int retries = 0; retries < 10; retries++) {
        // t = +1 s, request timed out, delay for 5 sec
        for (uint16_t i = 1; i <= expectedDelaySeconds; i++) {
          millis += 1000;
          fakeMillis->millis(millis);
          systemClock->runCoroutine();
          assertTrue(systemClock->isDelaying());
        }

        // t = +1 s, make another request and wait for response
        millis += 1000;
        fakeMillis->millis(millis);
        systemClock->runCoroutine();
        assertTrue(systemClock->isYielding());

        expectedDelaySeconds *= 2;
      }

      // Final delay of 3600
      expectedDelaySeconds = 3600;

      // t = +1 s, request timed out, delay for 5 sec
      for (uint16_t i = 1; i <= expectedDelaySeconds; i++) {
        millis += 1000;
        fakeMillis->millis(millis);
        systemClock->runCoroutine();
        assertTrue(systemClock->isDelaying());
      }

      // Make a new request and let it succeed
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

    FakeMillis* fakeMillis;
    FakeTimeKeeper* backupAndReferenceClock;
    TestableSystemClockCoroutine* systemClock;
};

testF(SystemClockCoroutineTest, sync) {
  assertRunCoroutine();
}

//---------------------------------------------------------------------------

void setup() {
#if defined(ARDUINO)
  delay(1000); // wait for stability on some boards to prevent garbage SERIAL_PORT_MONITOR
#endif
  SERIAL_PORT_MONITOR.begin(115200); // ESP8266 default of 74880 not supported on Linux
  while(!SERIAL_PORT_MONITOR); // for the Arduino Leonardo/Micro only
}

void loop() {
  TestRunner::run();
}
