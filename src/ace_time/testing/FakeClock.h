/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#ifndef ACE_TIME_FAKE_CLOCK_H
#define ACE_TIME_FAKE_CLOCK_H

#include <stdint.h>
#include "../clock/Clock.h"

namespace ace_time {
namespace testing {

/** A clock whose output can be controller for testing purposes. */
class FakeClock: public clock::Clock {
  public:
    void setNow(acetime_t epochSeconds) override {
      mEpochSeconds = epochSeconds;
    }

    acetime_t getNow() const override { return mEpochSeconds; }

    bool isResponseReady() const override { return mIsResponseReady; }

    void isResponseReady(bool ready) { mIsResponseReady = ready; }

  private:
    acetime_t mEpochSeconds = 0;
    bool mIsResponseReady = false;
};

}
}

#endif
