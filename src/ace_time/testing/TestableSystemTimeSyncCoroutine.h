#ifndef ACE_TIME_TESTABLE_SYSTEM_TIME_SYNC_COROUTINE_H
#define ACE_TIME_TESTABLE_SYSTEM_TIME_SYNC_COROUTINE_H

#include <AceRoutine.h>
#include <AceTime.h>
#include "FakeMillis.h"

namespace ace_time {
namespace testing {

class TestableSystemTimeSyncCoroutine:
    public provider::SystemTimeSyncCoroutine {
  public:
    TestableSystemTimeSyncCoroutine(
        provider::SystemTimeKeeper& systemTimeKeeper,
        FakeMillis* fakeMillis):
      SystemTimeSyncCoroutine(systemTimeKeeper),
      mFakeMillis(fakeMillis) {}

    unsigned long millis() const override {
      return mFakeMillis->millis();
    }

  private:
    FakeMillis* mFakeMillis;
};

}
}

#endif
