#ifndef ACE_TIME_TESTABLE_SYSTEM_CLOCK_SYNC_COROUTINE_H
#define ACE_TIME_TESTABLE_SYSTEM_CLOCK_SYNC_COROUTINE_H

#include <AceRoutine.h>
#include <AceTime.h>
#include "FakeMillis.h"

namespace ace_time {
namespace testing {

class TestableSystemClockSyncCoroutine:
    public clock::SystemClockSyncCoroutine {
  public:
    TestableSystemClockSyncCoroutine(
        clock::SystemClock& systemClock,
        FakeMillis* fakeMillis):
      SystemClockSyncCoroutine(systemClock),
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
