#ifndef ACE_TIME_FAKE_TIME_KEEPER_H
#define ACE_TIME_FAKE_TIME_KEEPER_H

#include <stdint.h>
#include "../provider/TimeKeeper.h"

namespace ace_time {
namespace testing {

class FakeTimeKeeper: public provider::TimeKeeper {
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
