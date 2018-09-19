#ifndef ACE_TIME_FAKE_SYSTEM_TIME_KEEPER_H
#define ACE_TIME_FAKE_SYSTEM_TIME_KEEPER_H

#include <stdint.h>
#include "../SystemTimeKeeper.h"

namespace ace_time {
namespace testing {

class FakeSystemTimeKeeper: public SystemTimeKeeper {
  public:
    explicit FakeSystemTimeKeeper(
            TimeProvider* syncTimeProvider /* nullable */,
            TimeKeeper* backupTimeKeeper /* nullable */):
        SystemTimeKeeper(syncTimeProvider, backupTimeKeeper) {}

    virtual unsigned long millis() const override {
      return mMillis;
    }

    void millis(unsigned long millis) {
      mMillis = millis;
    }

    unsigned long mMillis = 0;
};

}
}

#endif

