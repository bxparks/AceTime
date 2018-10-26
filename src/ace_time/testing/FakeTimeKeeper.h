#ifndef ACE_TIME_FAKE_TIME_KEEPER_H
#define ACE_TIME_FAKE_TIME_KEEPER_H

#include <stdint.h>
#include "../provider/TimeKeeper.h"

namespace ace_time {
namespace testing {

class FakeTimeKeeper: public provider::TimeKeeper {
  public:
    void setNow(uint32_t epochSeconds) override {
      mEpochSeconds = epochSeconds;
    }

    uint32_t getNow() const override { return mEpochSeconds; }

    bool isResponseReady() const override { return mIsResponseReady; }

    void isResponseReady(bool ready) { mIsResponseReady = ready; }

  private:
    uint32_t mEpochSeconds = 0;
    bool mIsResponseReady = false;
};

}
}

#endif
