/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#ifndef ACE_TIME_TESTABLE_SYSTEM_CLOCK_COROUTINE_H
#define ACE_TIME_TESTABLE_SYSTEM_CLOCK_COROUTINE_H

#include "../clock/SystemClockCoroutine.h"
#include "FakeMillis.h"

namespace ace_time {
namespace testing {

class TestableSystemClockCoroutine: public clock::SystemClockCoroutine {
  public:
    TestableSystemClockCoroutine(
        TimeProvider* referenceClock /* nullable */,
        TimeKeeper* backupClock /* nullable */,
        FakeMillis* fakeMillis):
      SystemClockCoroutine(referenceClock, backupClock),
      mFakeMillis(fakeMillis) {}

    unsigned long millis() const override {
      return mFakeMillis->millis();
    }

    unsigned long clockMillis() const override {
      return mFakeMillis->millis();
    }

  private:
    FakeMillis* mFakeMillis;
};

}
}

#endif
