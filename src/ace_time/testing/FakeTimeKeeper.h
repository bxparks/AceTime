#ifndef ACE_TIME_FAKE_TIME_KEEPER_H
#define ACE_TIME_FAKE_TIME_KEEPER_H

#include <stdint.h>
#include "../TimeKeeper.h"

namespace ace_time {
namespace testing {

class FakeTimeKeeper: public TimeKeeper {
  public:
    void setup() {}

    virtual void setNow(uint32_t secondsSinceEpoch) override {
      mSecondsSinceEpoch = secondsSinceEpoch;
    }

    virtual uint32_t getNow() const override {
      return mSecondsSinceEpoch;
    }

    uint32_t mSecondsSinceEpoch = 0;
};

}
}

#endif
