#ifndef ACE_TIME_FAKE_TIME_KEEPER_H
#define ACE_TIME_FAKE_TIME_KEEPER_H

#include <stdint.h>
#include "../provider/TimeKeeper.h"

namespace ace_time {
namespace testing {

class FakeTimeKeeper: public provider::TimeKeeper {
  public:
    void setNow(uint32_t secondsSinceEpoch) override {
      mSecondsSinceEpoch = secondsSinceEpoch;
    }

    uint32_t getNow() const override { return mSecondsSinceEpoch; }

    bool isResponseReady() const override { return mIsResponseReady; }

    void isResponseReady(bool ready) { mIsResponseReady = ready; }

  private:
    uint32_t mSecondsSinceEpoch = 0;
    bool mIsResponseReady = false;
};

}
}

#endif
