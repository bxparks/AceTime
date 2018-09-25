#line 2 "TimeKeeperTest.ino"

#include <AUnitVerbose.h>
#include <AceTime.h>
#include <ace_time/testing/FakeTimeKeeper.h>
#include <ace_time/testing/TestableSystemTimeKeeper.h>

using namespace aunit;
using namespace ace_time;
using namespace ace_time::common;
using namespace ace_time::testing;

class SystemTimeKeeperTest: public TestOnce {
  protected:
    virtual void setup() override {
      backupAndSyncTimeKeeper = new FakeTimeKeeper();
      systemTimeKeeper = new TestableSystemTimeKeeper(
          backupAndSyncTimeKeeper, backupAndSyncTimeKeeper);
      backupAndSyncTimeKeeper->setup();
      systemTimeKeeper->setup();
    }

    virtual void teardown() override {
      delete systemTimeKeeper;
      delete backupAndSyncTimeKeeper;
    }

    FakeTimeKeeper* backupAndSyncTimeKeeper; // backup and sync time keeper
    TestableSystemTimeKeeper* systemTimeKeeper;
};

testF(SystemTimeKeeperTest, setup) {
  assertEqual((uint32_t) 0, systemTimeKeeper->getNow());

  backupAndSyncTimeKeeper->setNow(100);
  systemTimeKeeper->setup();
  assertEqual((uint32_t) 100, systemTimeKeeper->getNow());
}

testF(SystemTimeKeeperTest, backupNow) {
  assertEqual((uint32_t) 0, systemTimeKeeper->getNow());
  assertEqual((uint32_t) 0, backupAndSyncTimeKeeper->getNow());

  unsigned long nowMillis = 1;
  systemTimeKeeper->millis(nowMillis);
  systemTimeKeeper->setNow(100);

  // setNow() caused a save to the backupTimeKeeper which happens to be the
  // same backupAndSyncTimeKeeper
  assertEqual((uint32_t) 100, systemTimeKeeper->getNow());
  assertEqual((uint32_t) 100, backupAndSyncTimeKeeper->getNow());
}

testF(SystemTimeKeeperTest, sync) {
  assertEqual((uint32_t) 0, systemTimeKeeper->getNow());
  assertEqual((uint32_t) 0, systemTimeKeeper->getLastSyncTime());
  assertEqual((uint32_t) 0, backupAndSyncTimeKeeper->getNow());

  unsigned long nowMillis = 1;
  systemTimeKeeper->millis(nowMillis);
  systemTimeKeeper->sync(100);

  // sync() does NOT write to backupTimeKeeper because the sync and backup
  // sources are identical, so it does not try to write the time back into
  // itself.
  assertEqual((uint32_t) 100, systemTimeKeeper->getNow());
  assertEqual((uint32_t) 100, systemTimeKeeper->getLastSyncTime());
  assertEqual((uint32_t) 0, backupAndSyncTimeKeeper->getNow());
}

testF(SystemTimeKeeperTest, getNow) {
  unsigned long nowMillis = 1;

  systemTimeKeeper->millis(nowMillis);
  systemTimeKeeper->setNow(100);
  assertEqual((uint32_t) 100, systemTimeKeeper->getNow());

  // +900ms, no change to getNow()
  nowMillis += 900;
  systemTimeKeeper->millis(nowMillis);
  assertEqual((uint32_t) 100, systemTimeKeeper->getNow());

  // +100ms, getNow() should increase by 1
  nowMillis += 100;
  systemTimeKeeper->millis(nowMillis);
  assertEqual((uint32_t) 101, systemTimeKeeper->getNow());

  // +30000ms, getNow() should increase by 30
  nowMillis += 30000;
  systemTimeKeeper->millis(nowMillis);
  assertEqual((uint32_t) 131, systemTimeKeeper->getNow());

  // +40000ms, causing rollover of internal uint16_t version of millis, but
  // getNow() should still increase by another 40
  nowMillis += 40000;
  systemTimeKeeper->millis(nowMillis);
  assertEqual((uint32_t) 171, systemTimeKeeper->getNow());
}

// --------------------------------------------------------------------------

void setup() {
  delay(1000); // wait for stability on some boards to prevent garbage Serial
  Serial.begin(115200); // ESP8266 default of 74880 not supported on Linux
  while(!Serial); // for the Arduino Leonardo/Micro only
}

void loop() {
  TestRunner::run();
}
