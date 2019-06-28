/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#ifndef ACE_TIME_TESTABLE_SYSTEM_CLOCK_H
#define ACE_TIME_TESTABLE_SYSTEM_CLOCK_H

#include <stdint.h>
#include "../clock/SystemClock.h"

namespace ace_time {
namespace testing {

/**
 * A version of SystemClock that allows the millis() function to be
 * manually set for testing purposes.
 */
class TestableSystemClock: public clock::SystemClock {
  public:
    explicit TestableSystemClock(
            TimeProvider* syncTimeProvider /* nullable */,
            TimeKeeper* backupTimeKeeper /* nullable */,
            FakeMillis* fakeMillis):
        SystemClock(syncTimeProvider, backupTimeKeeper),
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

