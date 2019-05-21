#line 2 "TimeKeeperTest.ino"

#include <AUnitVerbose.h>
#include <AceRoutine.h> // enable SystemTimeSyncCoroutine
#include <AceTime.h>
#include <ace_time/testing/FakeMillis.h>
#include <ace_time/testing/FakeTimeKeeper.h>
#include <ace_time/testing/TestableSystemTimeKeeper.h>
#include <ace_time/testing/TestableSystemTimeSyncCoroutine.h>

using namespace aunit;
using namespace ace_time;
using namespace ace_time::provider;
using namespace ace_time::common;
using namespace ace_time::testing;

class SystemTimeKeeperTest: public TestOnce {
  protected:
    void setup() override {
      fakeMillis = new FakeMillis();
      backupAndSyncTimeKeeper = new FakeTimeKeeper();
      systemTimeKeeper = new TestableSystemTimeKeeper(
          backupAndSyncTimeKeeper, backupAndSyncTimeKeeper, fakeMillis);

      systemTimeKeeper->setup();
    }

    void teardown() override {
      delete systemTimeKeeper;
      delete backupAndSyncTimeKeeper;
      delete fakeMillis;
    }

    FakeMillis* fakeMillis;
    FakeTimeKeeper* backupAndSyncTimeKeeper; // backup and sync time keeper
    TestableSystemTimeKeeper* systemTimeKeeper;
};

testF(SystemTimeKeeperTest, setup) {
  assertEqual((acetime_t) 0, systemTimeKeeper->getNow());

  backupAndSyncTimeKeeper->setNow(100);
  systemTimeKeeper->setup();
  assertEqual((acetime_t) 100, systemTimeKeeper->getNow());
}

testF(SystemTimeKeeperTest, backupNow) {
  assertEqual((acetime_t) 0, systemTimeKeeper->getNow());
  assertEqual((acetime_t) 0, backupAndSyncTimeKeeper->getNow());

  unsigned long nowMillis = 1;
  fakeMillis->millis(nowMillis);
  systemTimeKeeper->setNow(100);

  // setNow() caused a save to the backupTimeKeeper which happens to be the
  // same backupAndSyncTimeKeeper
  assertEqual((acetime_t) 100, systemTimeKeeper->getNow());
  assertEqual((acetime_t) 100, backupAndSyncTimeKeeper->getNow());
}

testF(SystemTimeKeeperTest, sync) {
  assertEqual((acetime_t) 0, systemTimeKeeper->getNow());
  assertEqual((acetime_t) 0, systemTimeKeeper->getLastSyncTime());
  assertEqual((acetime_t) 0, backupAndSyncTimeKeeper->getNow());

  unsigned long nowMillis = 1;
  fakeMillis->millis(nowMillis);
  systemTimeKeeper->sync(100);

  // sync() does NOT write to backupTimeKeeper because the sync and backup
  // sources are identical, so it does not try to write the time back into
  // itself.
  assertEqual((acetime_t) 100, systemTimeKeeper->getNow());
  assertEqual((acetime_t) 100, systemTimeKeeper->getLastSyncTime());
  assertEqual((acetime_t) 0, backupAndSyncTimeKeeper->getNow());
}

testF(SystemTimeKeeperTest, getNow) {
  unsigned long nowMillis = 1;

  fakeMillis->millis(nowMillis);
  systemTimeKeeper->setNow(100);
  assertEqual((acetime_t) 100, systemTimeKeeper->getNow());

  // +900ms, no change to getNow()
  nowMillis += 900;
  fakeMillis->millis(nowMillis);
  assertEqual((acetime_t) 100, systemTimeKeeper->getNow());

  // +100ms, getNow() should increase by 1
  nowMillis += 100;
  fakeMillis->millis(nowMillis);
  assertEqual((acetime_t) 101, systemTimeKeeper->getNow());

  // +30000ms, getNow() should increase by 30
  nowMillis += 30000;
  fakeMillis->millis(nowMillis);
  assertEqual((acetime_t) 131, systemTimeKeeper->getNow());

  // +40000ms, causing rollover of internal uint16_t version of millis, but
  // getNow() should still increase by another 40
  nowMillis += 40000;
  fakeMillis->millis(nowMillis);
  assertEqual((acetime_t) 171, systemTimeKeeper->getNow());
}

//---------------------------------------------------------------------------

// Create dedicated test class to allow friend access to private members of
// SystemTimeSyncCoroutine for testing purposes.
class SystemTimeSyncCoroutineTest: public TestOnce {
  protected:
    void setup() override {
      fakeMillis = new FakeMillis();
      backupAndSyncTimeKeeper = new FakeTimeKeeper();
      systemTimeKeeper = new TestableSystemTimeKeeper(
          backupAndSyncTimeKeeper, backupAndSyncTimeKeeper, fakeMillis);
      systemTimeSyncCoroutine = new TestableSystemTimeSyncCoroutine(
          *systemTimeKeeper, fakeMillis);

      systemTimeKeeper->setup();
      systemTimeSyncCoroutine->setupCoroutine("systemTimeSyncCoroutine");
    }

    void teardown() override {
      delete systemTimeSyncCoroutine;
      delete systemTimeKeeper;
      delete backupAndSyncTimeKeeper;
      delete fakeMillis;
    }

    void assertRunCoroutine() {
      unsigned long millis = 0;
      backupAndSyncTimeKeeper->isResponseReady(false);

      // t = 0, sends request and waits for response
      systemTimeSyncCoroutine->runCoroutine();
      assertTrue(systemTimeSyncCoroutine->isYielding());

      // retry with exponential backoff 10 times, doubling the delay on each
      // iteration
      uint16_t expectedDelaySeconds = 5;
      for (int retries = 0; retries < 10; retries++) {
        // t = +1 s, request timed out, delay for 5 sec
        for (uint16_t i = 1; i <= expectedDelaySeconds; i++) {
          millis += 1000;
          fakeMillis->millis(millis);
          systemTimeSyncCoroutine->runCoroutine();
          assertTrue(systemTimeSyncCoroutine->isDelaying());
        }

        // t = +1 s, make another request and wait for response
        millis += 1000;
        fakeMillis->millis(millis);
        systemTimeSyncCoroutine->runCoroutine();
        assertTrue(systemTimeSyncCoroutine->isYielding());

        expectedDelaySeconds *= 2;
      }

      // Final delay of 3600
      expectedDelaySeconds = 3600;

      // t = +1 s, request timed out, delay for 5 sec
      for (uint16_t i = 1; i <= expectedDelaySeconds; i++) {
        millis += 1000;
        fakeMillis->millis(millis);
        systemTimeSyncCoroutine->runCoroutine();
        assertTrue(systemTimeSyncCoroutine->isDelaying());
      }

      // Make a new request and let it succeed
      millis += 1000;
      fakeMillis->millis(millis);
      backupAndSyncTimeKeeper->isResponseReady(true);
      backupAndSyncTimeKeeper->setNow(42);

      // verify successful request
      systemTimeSyncCoroutine->runCoroutine();
      assertTrue(systemTimeSyncCoroutine->isDelaying());
      assertEqual(systemTimeSyncCoroutine->mRequestStatus,
          SystemTimeSyncCoroutine::kStatusOk);
      assertEqual((acetime_t) 42, systemTimeKeeper->getNow());
      assertEqual((acetime_t) 42, systemTimeKeeper->getLastSyncTime());
      assertTrue(systemTimeKeeper->isInit());
    }

    FakeMillis* fakeMillis;
    FakeTimeKeeper* backupAndSyncTimeKeeper;
    TestableSystemTimeKeeper* systemTimeKeeper;
    SystemTimeSyncCoroutine* systemTimeSyncCoroutine;
};

testF(SystemTimeSyncCoroutineTest, sync) {
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
